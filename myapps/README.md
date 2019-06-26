# Fake News Analysis

Core provenance library (CPL): https://github.com/margoseltzer/prov-cpl
Article scraping: https://github.com/margoseltzer/article-scraping

Before running, please setup/install the CPL. 
The article scraping scripts can be used to extract data from articles and populate the CPL.

**To compile and run vertex relabeling (from the myapps directory):**
```
    make
    ./main
```

**To specify the file you want graph data to be written to (default file name is "data.txt"):**
```
    ./main file <graph name>
```

**To specify the number of iterations (default # iterations is 4):**
```
    ./main niters <number of iterations>
```

**To specify a subset of bundles (default is to analyze all bundles in the cpl):**
```
    ./main bundles <bundle_file>
```
The bundle file should contain a list of bundle IDs, separated by white space. See ``example_bundle_file.txt``. An exception will be thrown if the bundle file has an invalid bundle ID.


**To clean (from myapps directory):**
```
    make clean
```

## Notes:
* When running the program, graphchi may prompt you to enter the filetype. Choose ``edgelist``.
* You may need to define the ``GRAPHCHI_ROOT`` environment variable. This should be set as the fake-news directory.