# LibVT
LibVT is a library implementing "virtual texturing". LibVT itself is written in C++ but exposes a C-based API.




Download a  Demo App Binary for Win/Mac to test out LibVT:

[Demo App Binary](https://bintray.com/artifact/download/corecode/LibVT/libvt_demo_binaries_win32_mac.zip)

(The demo also contains a free 32k^2 virtual texture dataset which you can use for for testing your LibVT project)


The theory behind LibVT:
[Virtual Texturing Thesis](https://www.cg.tuwien.ac.at/research/publications/2010/Mayer-2010-VT/)



**Features:
**

*  NEW: LibVT is now completely MIT-licensed
*  Implements "virtual texturing" in the style of Sean Barrett and John Carmack
*  Using OpenGL with shaders in GLSL or Cg.
*  Designed as a library to allow easy integration into existing rendering engines (e.g., OpenSceneGraph integration has been done).
*  Compatible with OpenGL ES 2.0 and support for iOS (iPhone / iPad).
*  Tile determination in view space using a read-back.
*  Support for bilinear, trilinear and anisotropic filtering.
*  Configurability of physical texture dimension, RAM-cache size, tile border, tile- size, virtual texture size, resident mipmap-levels, cache warming, etc.
*  Mipmap-chain length of up to 11, allowing a virtual texture resolution of 256k^2 with 256^2 pixel tiles.
*  Multiple tile decompression libraries: LibPNG, LibJPEG, LibJPEG-Turbo, STBI , ImageMagick and CoreGraphics.
*  Usage of compressed(JPEG,PNG,etc), uncompressed or DXT1/5 pre-compressed tiles.
*  Multithreaded and decoupled tile streaming using boost::thread and with optional real-time DXT compression using FastDXT.
*  All texture transfers (read-back, pagetable texture and physical texture) optionally asynchronous using PBOs.
*  Either stores fallback entries in the pagetable texture or uses looping in the frag- ment shader.
*  Optional dynamic adjustment of the LoD bias to fit visible tiles into the physical texture.
*  LibVT also includes a pipeline for generating virtual textures out of individual texture files: generateTextureAtlas, generateVirtualTextureTiles, mergeVirtualTextureTiles, convertVirtualTextureTiles, offsetObjTexcoords