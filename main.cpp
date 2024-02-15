/*
This program helps user capture images for purpose of image acquisition in order to calculate MIG (Mean Intensity Gradient) before performing testing.
*/

#include <iostream>
#include <filesystem>
#include <opencv4/opencv2/opencv.hpp>
#include "VmbCPP/VmbCPP.h"
#include "xlsxwriter.h"

using namespace VmbCPP;

/*
This function creates folders.

func: create_folders()
param: path relative to build folder
return: 0 or 1
*/
int create_folders(const std::string &path);

int main()
{
    VmbSystem &system = VmbSystem::GetInstance();

    VmbErrorType err;
    FeaturePtr feature;
    CameraPtrVector cameras;
    StreamPtrVector streams;
    FramePtr frame;
    VmbUint32_t frame_h, frame_w;
    VmbUint32_t t_out = 50; // timeout in miliseconds to allow frame to be filled
    VmbUint64_t time_stamp;
    VmbUchar_t *pImage = nullptr;
    double exposure_time, gain, blackLvl, gamma, frame_rate, frame_rate_limit;
    const int ENTER_KEY_CODE = 13;
    
    // Check if API starts or not
    err = system.Startup();
    if (err != VmbErrorSuccess)
    {
        std::cout << "Could not start the API." << std::endl;
        return EXIT_FAILURE;
    }

    // Get a list of connected cameras
    err = system.GetCameras(cameras);
    if (err != VmbErrorSuccess)
    {
        std::cout << "No cameras found." << std::endl;
        system.Shutdown();
        return EXIT_FAILURE;
    }

    // Allow full access to the camera
    err = cameras[0]->Open(VmbAccessModeFull);
    if (err != VmbErrorSuccess)
    {
        std::cout << "Cannot access the cameras." << std::endl;
        return EXIT_FAILURE;
    }

    // Allow streaming from the camera
    err = cameras[0]->GetStreams(streams);
    if (err != VmbErrorSuccess)
    {
        std::cout << "Not able to stream." << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "\n///////////////////////////////\n"
              << "//// Printing general info ////\n"
              << "///////////////////////////////\n" <<std::endl;

    // Get Exposure Time          
    cameras[0]->GetFeatureByName("ExposureTimeAbs", feature);
    feature->GetValue(exposure_time);
    std::cout << "/// Exposure Time (Before)        :        " << exposure_time << " us" << std::endl;

    // Set Exposure Time
    exposure_time = 3000.0; // in microseconds
    cameras[0]->GetFeatureByName("ExposureTimeAbs", feature);
    feature->SetValue(exposure_time);
    std::cout << "/// Exposure Time (After)         :        " << exposure_time << " us" << std::endl;

    // Get Gain
    cameras[0]->GetFeatureByName("Gain", feature);
    feature->GetValue( gain );
    std::cout << "/// Gain (Before)                 :        " << gain << std::endl;

    // Set Gain
    gain = 0.0;
    cameras[0]->GetFeatureByName("Gain", feature);
    feature->SetValue(gain);
    std::cout << "/// Gain (After)                  :        " << gain << std::endl;

    // Get Black Level
    cameras[0]->GetFeatureByName("BlackLevel", feature);
    feature->GetValue(blackLvl);
    std::cout << "/// Black Level       :        " << blackLvl << std::endl;

    // Get Gamma
    cameras[0]->GetFeatureByName("Gamma", feature);
    feature->GetValue(gamma);
    std::cout << "/// Gamma             :        " << gamma << std::endl;

    // Get current frame rate
    cameras[0]->GetFeatureByName("AcquisitionFrameRateAbs", feature);
    feature->GetValue(frame_rate);
    std::cout << "/// Frame Rate        :        " << frame_rate << " fps" << std::endl;

    // Get max. possible frame rate
    cameras[0]->GetFeatureByName("AcquisitionFrameRateLimit", feature);
    feature->GetValue(frame_rate_limit);
    std::cout << "/// Frame Rate Limit  :        " << frame_rate_limit << " fps" << std::endl;
    std::cout << "\n///////////////////////////////\n"
              << "///////////// Done ////////////\n"
              << "///////////////////////////////\n" << std::endl;

    std::string root_path = "../images";
    create_folders(root_path); // Create root folder
    
    std::string folder_path = root_path + "/" + "Gain_" + std::to_string(int(gain)) + "_Exposure_" + std::to_string(int(exposure_time));
    create_folders(folder_path); // Create folder for the given exposure time and gain
    
    std::string timestamp_folder = "../timestamps";
    create_folders(timestamp_folder); // Create a timestamp folder

    std::string file_path, xlsx_path;
    xlsx_path = timestamp_folder + "/" + "Gain_" + std::to_string(int(gain)) + "_Exposure_" + std::to_string(int(exposure_time)) + ".xlsx";
    int frame_count = 0;

    lxw_workbook *workbook = workbook_new(xlsx_path.c_str());
    lxw_worksheet *worksheet = workbook_add_worksheet(workbook, NULL);

    worksheet_write_string(worksheet, 0, 0, "Timestamp (ns)", NULL);

    while (true)
    {
        // Acquire a single frame
        err = cameras[0]->AcquireSingleImage(frame, t_out);
        if (err == VmbErrorSuccess)
        {
            // Get height of the frame
            err = frame->GetHeight(frame_h);
            if (err != VmbErrorSuccess)
            {
                std::cout << "Failed to get frame height!\n" <<std::endl; 
            }

            // Get width
            err = frame->GetWidth(frame_w);
            if (err != VmbErrorSuccess)
            {
                std::cout << "Failed to get frame width!\n" <<std::endl; 
            }

            // Get image data
            err = frame->GetImage(pImage);
            if (err != VmbErrorSuccess)
            {
                std::cout << "Failed to acquire image data. \n" << std::endl; 
            }

            // Get timestamp of the acquired image            
            err = frame->GetTimestamp(time_stamp);
            if (err != VmbErrorSuccess)
            {
                std::cout << "Failed to acquire timestamp. \n" << std::endl; 
            }

            // Convert image data to OpenCV format and save it to the given path
            cv::Mat cvMat(frame_h, frame_w, CV_8UC1, pImage);
            file_path = folder_path + "/frame_" + std::to_string(frame_count) + ".png";
            cv::imwrite(file_path, cvMat);
            cv::imshow("Frame Window (Press 'Enter' to quit)", cvMat);

            worksheet_write_number(worksheet, (frame_count + 1), 0, time_stamp, NULL);
            frame_count++;

            // Press enter to exit program
            if (cv::waitKey(1) == ENTER_KEY_CODE)
            {
                break;
            }
        }
    }

    cv::destroyAllWindows();
    cameras[0]->Close();
    system.Shutdown();
    workbook_close(workbook);
    
    return EXIT_SUCCESS;
}

int create_folders(const std::string &path)
{
    // If the folder(s) doesn't exist, create it
    if (!std::filesystem::exists(path))
    {
        std::cout << "\n//////////////////////////////////////////////////////////////\n"
                  << "/// Creating folder at    :       "   << path
                  << "\n//////////////////////////////////////////////////////////////\n"
                  << std::endl;
        std::filesystem::create_directories(path);
        return EXIT_SUCCESS;
    } else
    {
        std::cout << "\n//////////////////////////////////////////////////////////////\n"
                  << "/// Folder already at     :       "   << path
                  << "\n//////////////////////////////////////////////////////////////\n"
                  << std::endl;
        return EXIT_FAILURE;
    }
}