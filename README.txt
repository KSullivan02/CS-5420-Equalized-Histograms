--------Project 4 - Equalized Histograms---------
Usage: ./Histogram -h -m[mode number] inputImage compareImage fileHistogram
-h displays usage information and help menu
-m mode of eqalization numbering from 1- compute histogram and equalize it. 2- compare from another image and equalize based on it. 3- equalize from a saved histogram text file
inputImage: The iamge to be Equalized
compareImage: if in mode 2, use the comparedImage to equalize the inputImage
fileHistogram: If in mode 3, use this collection of 256 entries of a histogram to equalize the inputImage.