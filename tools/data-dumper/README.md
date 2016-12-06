# Description
This tool is to take test images from different dataset and dump as a 2D array into a C source file. This is useful to test the bare-metal CNN applicaitons for accuracy purpose. You can also specify the linker attributes if any that you want to put for the big array that is created. Ex. You can specify the alignment and the RAM segment that you want to specify for the array.

# Dependencies
- Pylearn2
	You need to install pylearn2 and also download datasets. After that you need to set the PYLEARN2_DATA_PATH variable to the data directory. The datasets can be downloaded by running different scripts in  **pylearn2/pylearn2/scripts/datasets** directory. Set the above variable before downloading so that the script will download the datasets in correct directory. More instructions on using Pylearn2 can be found [here](http://deeplearning.net/software/pylearn2/)
- Numpy

# To run
pyhton  *dataset_dumper.py* --dset= `<dataset_name`> --nimg=`<number_of_images_to_dump`>
Run **'python dataset_dumper.py --help'**   for help

The tool will create  C and H files that you need to include in the target project.
# Features
1. Specify Linker Attributes
To specify linker attributes to the created array, modify **data_linker_attr**  and **label_linker_attr** variable in the script.
2. Add more datasets
To add more dataset support, add the dataset name to the dictionary **dataset_factory** and add one more case in **_get_dataset** method to read the data and labels for  the new dataset. Note that the returned data array must be a 2D array.
