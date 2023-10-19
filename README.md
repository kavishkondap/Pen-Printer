# Pen Printer Project
This project was recognized with 2nd place in the 2022 3D Experience Project of the Year Competition. We appreciate the support from everyone who contributed to our success. For more information on the competition, please visit here: https://poty.edu.3ds.com/.

## Project Overview
Resolution: The printer offers a resolution of 3840 x 2160, thanks to precision stepper motors with an accuracy of up to 0.1125 degrees in the X and Y axes. Even with a 0.4 millimeter pen tip, it delivers resolutions of 635 x 476, ensuring the creation of high-quality images.

Movements: Utilizing stepper motors with GT2 timing belts wrapped around pulleys ensures swift movements, preserving the desired precision of a CNC machine. This approach offers a significant advantage over leadscrew mechanisms in terms of speed and accuracy.

Manufacturing Techniques: The entire assembly was  designed in SOLIDWORKS, incorporating a range of manufacturing techniques, including CNC machining, manual machining, 3D printing for additive manufacturing, and laser cutting.

Z-Axis Mechanism: The CNC's Z-axis employs a custom-designed rack and pinion mechanism created in SOLIDWORKS using equation-driven curves. Stress analysis was conducted using SOLIDWORKS Simulation software to ensure structural integrity under cantilevered loads.

## Image Processing
Following the assembly's construction, we developed an image processing program using Python. This program takes camera input and generates a list of instructions for the printer. We leverage the OpenCV library to process images, using a Gaussian threshold image processing algorithm to convert colorful images into black and white representations of where dots should be placed on the page.

## Path Generation
The black and white image is treated as a list of points, which then undergoes a closest-first path generation algorithm. This algorithm efficiently converts the path into instructions for the Arduino, including adjustments for pen height based on the cantilever's drop as it moves horizontally.
