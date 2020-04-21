# Spectrum

A simple spectrum analyzer for Windows that uses FFTW

## Getting Started

This is a simple Windows application written for RADStudio/C++Builder which, using the FFWT library, displays the spectrum of a signal from an audio device. The spectrum graph can be chosen between two modes: logarithmic (dB) or linear.

<img src="https://i.ibb.co/wMZTb4r/1-611-D020-B-2-C12-4839-8567-A7-E8-A650940-E.png" alt="Figure 1">

### Prerequisites

You need C++Builder 10.3.3 edition (e.g. the <a href="https://www.embarcadero.com/products/cbuilder/starter">community edition</a>) and <a href="http://fftw.org/install/windows.html">FFTW DLLs</a>.

You also need boost libraries (1.68.0) which you can easily install using GetIt.

<img src="https://i.ibb.co/FmPznnX/3-611-D020-B-2-C12-4839-8567-A7-E8-A650940-E.png" alt="Figure 2">

### Installing

After cloning the repository, you need to download the precompiled <a href="http://fftw.org/install/windows.html">FFTW DLLs</a> (both 32 and 64 bit versions).

Next you need to create a folder called FFTW in the project root. In turn, within the FFTW folder, two other folders must be created, respectively with the name fftw-3.3-dll32 and fftw-3.3-dll64. Now you will have to unzip the contents of the fftw-3.3.x-dll32.zip archive in the fftw-3.3-dll32 folder and the contents of the fftw-3.3.x-dll64.zip archive in the fftw-3.3-dll64 folder.

(note that 'x' in the name of the FFTW archive depends on the FFTW version you download)

For example:

```
E:\Prj\Spectrum>md FFTW

E:\Prj\Spectrum>cd FFTW

E:\Prj\Spectrum\FFTW>md fftw-3.3-dll32

E:\Prj\Spectrum\FFTW>md fftw-3.3-dll64

E:\Prj\Spectrum\FFTW>powershell Expand-Archive -Path %userprofile%\Downloads\fftw-3.3.x-dll32.zip -DestinationPath fftw-3.3-dll32

E:\Prj\Spectrum\FFTW>powershell Expand-Archive -Path %userprofile%\Downloads\fftw-3.3.x-dll64.zip -DestinationPath fftw-3.3-dll64

E:\Prj\Spectrum>tree
+---anafestica
+---FFTW
¦   +---fftw-3.3-dll32
¦   +---fftw-3.3-dll64
+---Libs
¦   +---Win32
¦   +---Win64
+---Resources
```

Depending on whether you have chosen to compile a 32 or 64 bit application, the build process will copy the appropriate DLL to the executable folder from the fftw-3.3-dll32 folder or from the fftw-3.3-dll64 folder using the following script saved in the Project Options' Build Events.

<img src="https://i.ibb.co/HHQspRB/4-611-D020-B-2-C12-4839-8567-A7-E8-A650940-E.png" alt="Figure 3">

<img src="https://i.ibb.co/FwFYxwP/2-611-D020-B-2-C12-4839-8567-A7-E8-A650940-E.png" alt="Figure 4">

## License

This project is licensed under the "The Unlicense".