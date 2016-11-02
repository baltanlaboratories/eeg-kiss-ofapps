# eeg-kiss-ofapps
Hack the body

This repository contains the OpenFrameworks based visualization application that is used in the EEG-Kiss project.

To get it up and running, a numebr of things need to be in place:
- We used Visual Studio Community 2015 to build it. Installing it and opening the solution file should be sufficient to get the right compilers.
- The solution file expects OpenFrameworks to be installed in C:\openframeworks. For the current version, we used OF 0.9.4. (Find the donwload OF for Visual Studio on Windows at http://openframeworks.cc/download/)
- To run the application on another computer, two things need to be kept in mind: 1) compile a release version, not a debug version and 2) it may be necesarry to install the Visual C++ redistributable for VS 2015 (https://www.microsoft.com/en-us/download/details.aspx?id=48145). (This may be common C++ knowledge, but I needed to figure it out, so I just put it down here as well) 
