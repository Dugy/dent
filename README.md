# dent
Software for processing data from nanoindentation experiments written in C++. The GUI is made via Qt.

Features
--
- Loads exported data from Hysitron (might need small edits to parse other file types)
- Filters the data, removing random noise but preserving pop-ins that have a physical basis
- Computes the hardness of the material
- Computes the work done to penetrate to that depth
- Fits the data into user-given formulae
- Can process many files at once
- Shows a graphic preview of the results
- Work is multithreaded
- Uses only core C++ libraries and Qt, so it should work on any platform (tested: Windows, Linux)
