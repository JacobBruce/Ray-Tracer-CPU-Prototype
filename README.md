# Ray-Tracer-CPU-Prototype

Pray Engine is an experimental ray tracing engine which is capable of rendering analytical spheres and triangle polygons with textures. It includes features such as support for normal maps, surface transparency, colored shadows, and some basic lighting models.

It is a VS 2012 project and uses a DirectX 9 window to display the rendered result. However it doesn't use the GPU to compute anything, my next goal is to use OpenCL to handle all the parallel processing by writing totally custom kernels based on this prototype code.

Keep in mind this is only a prototype engine designed to run on the CPU. Right now it's mainly written for readability and doesn't use SSE, multi-threading, or anything else to speed it up, and the default scene isn't large enough to benefit from any of the optimization techniques used.

It is set up to load a generic scene containing a simple room and a matchbox floating in the center of the room. You can move around using the WASD keys like a normal game, but you can also tilt the camera with Q and E, and move up and down using the page up and page down keys.

When I get some free time I'll try to upload specifications for all the engine file formats. For now just check the source code if you wish to know how it works. I also made an app for converting OBJ files to the Pray Engine format if anybody is interested.

Since some of the textures in the default scene are rather large and may be copyrighted I decided not to upload them here. After downloading the archive, extract the Data folder to your project folder or the same place as the exe file and you should be ready to go.

Link to download Data folder: 
https://mega.nz/#!ZEoyRT4D!j-i_5jlACI0NJoZz32-cJk86Sg91TnmI7T1Y2lmwxKU

You can find a description of the methods used in this engine here: www.j-d-b.net/files/Real-Time_Ray-Tracing_Methodology.pdf

Pray Engine is based on the Chili DirectX Framework:
http://www.planetchili.net/