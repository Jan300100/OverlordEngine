OverLordEngine Devlog

Day 1: 26 february 2023 : Found this old project gathering dust on my HDD. Time to revisit it. when launching the game. first thing I notice is the Noisy image, and bad performance. added mipmaps and changed texture filtering method. to clear up a lot of the noise in the image. and the mipmaps also gave a nice boost to performance, pretty much doubling our framerate and taking us from 40fps to 80fps. just a few lines of code and I would have had a better grade for this project..

Day 2 : 27 february : In order to use PIX or NSight, The project needs to be build in X64. But Its only configuration is x86. Time to go grab all the libraries it uses, FMOD, physx 3.4, DirectXTex and DirectXEffects, and build the x64 versions. clear up some errors and build the game and engine in x64.

Day 3 : 28 february : PIX doesn’t seem to like Directx11 very much. but I can already add some pix events for the cpu code. which showed me the bottleneck is the gpu. this was surprising, as I was getting worried about the cpu, because there was a lot of code I had written as a student that was suboptimal to say the least. It would be nice to get some gpu captures and gpu events. but the gpu captures don’t seem to work at the moment, and gpu events aren’t very friendly when using directx11. So the idea starts to grow to try and port the engine to modern graphics api. first Directx12, and while we are at it, maybe Vulkan. But that idea needs to ferment a little more before I can start on it. I don’t want to dive in blindly. So before I do the porting, it’s time for some source control. Uploaded the code to github so I don’t have to worry about unreversibly damaging the engine with all my shenanigans ....

Day 4: 1 march Fixing a grass issue where black pixels appeared on the foliage. 3 reasons: 1st reason: textures containing lots of black and had to be filled with appropriate color. second reason was a bug in the pixel shader, where the dot product between normal and lightdirection was negative, the final color turned to 0.
3rd reason: backfacing foliage having the wrong normal.

Day 5: 10 march : back to porting the game to directx12. first of all. I need to abstract away all the directx11 code. and replace it with some intermediary format. 
for now, I will stick with DirectX11, as it will give me the chance to go over the entire engine, and get an idea of what is there and what is going on. by the time I'll be done.
I'll have a good understanding of the engine and what I should think about when creating the Directx12 implementation of the interface.

First thing to abstract is the device and devicecontext. 
then I'll go to resources.