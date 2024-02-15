# Description
- This repo contains source code to perform initial image acquisition.
- It saves the captured images inside the 'images' folder.
- The timestamps are saved inside the 'timestamps' folder.

# Installation
- Go to project directory and create folders `lib` and `build`:
  ```
  cd <your_path>/init_img_acq
  mkdir -p build lib
  ```

- VimbaX Library
  - Download the `VmbCPP` library `VimbaX_Setup-2023-1-Linux64.tar.gz` from [here](https://www.alliedvision.com/en/products/software/vimba-x-sdk/).
  - Extract the contents of tar file: `tar -xvzf <your_download_path>/VimbaX_Setup-2023-1-Linux64.tar.gz`
  - Paste the extracted directory inside the `lib` folder.

- Install ZLIB: `sudo apt install zlib1g-dev`
- Install LibXLSXWriter from instructions [here](https://libxlsxwriter.github.io/getting_started.html). Make it on your own from given instructions inside your `lib` directory. 

# Build
```
# Go to build folder
cd build

# Generate makefiles
cmake -s ..

make -j

# Run the executable
./init_img_acq
```

# Troubleshooting for Camera API
  ```
  # Ping the camera IP address
  ping <camera_ip_address>

  # Run VimbaXViewer
  cd lib/VimbaX_2023-1/bin
  ./VimbaXViewer
  
  # If TL (Transport Layer) not found
  cd ../cti
  sudo -E ./SetGenTLPath.sh
  sudo -E ./VimbaGigETL_Install.sh

  # Log off your system and run VimbaXViewer again after restart
  ```