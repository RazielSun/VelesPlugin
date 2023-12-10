# VelesPlugin
Veles, also known as Volos, is a major god of earth, waters, livestock, and the underworld in Slavic paganism. [Wikipedia](https://en.wikipedia.org/wiki/Veles_(god))

## Information
This plugin is an education project based on video [GPU-Based Run-Time Procedural Placement in Horizon: Zero Dawn](https://youtu.be/ToCozpl1sYY)
I tried to reproduce [this cool part](https://youtu.be/ToCozpl1sYY?t=998) from video.

## Education program
### Layers system [World Data](https://youtu.be/ToCozpl1sYY?t=513)
- I have a Paint Layer for custom drawing. [Painted World Data](https://youtu.be/ToCozpl1sYY?t=609)
- As a mask I can use Weightmaps from landscape
- A World Data Layer is for generated specific maps (for example I tried to use Houdini to generate some mask in my test project)

### Density map [Here](https://youtu.be/ToCozpl1sYY?t=1448)
- Texture and source type (layer or weightmap)
- A few scalar parameters for composing
- I think about how I can optimize it (texture array)
- Layer texture without mips (Horizon uses high mip for trees and lower for grass)
- Different sizes (Horizon 64px per block)

### Patterns
- I tried 'Dither algorithm' and it's very interesting. But I didn't include it. [Here](https://youtu.be/ToCozpl1sYY?t=1576)
- I have basics algorithms (Grid, Random and Blue Noise, Random Diamond variation)
- I didn't fully understand what they do [here](https://youtu.be/ToCozpl1sYY?t=1680) with footprint. It's beautiful.
- I generate several wavefronts and use a pattern as stamp [like here](https://youtu.be/ToCozpl1sYY?t=1844)

### Placement
- I use idx from wavefront for random, they used [stencil? mask](https://youtu.be/ToCozpl1sYY?t=2029)
- I decided to use 3 vector4 instead matrix, maybe in the future..

### Unreal GPU Compute Shaders
- I used RDG (Render Dependency Graph) and didn't separate it
- They have production pipeline, more complex and with dependencies [Here](https://youtu.be/ToCozpl1sYY?t=2437)

### Spawning system
- From readback I move data to Array of instances
- I use HISMC with Actors per tile. I don't support World Partition Actors, but mb in the future..

### Solving collision?
- I didn't create 'solving collision' system from [here](https://youtu.be/ToCozpl1sYY?t=2234)

## Known issues
- Heightfield Z location from texture isn't correct when we have slopes
- Don't use a normal vector from a landscape texture
