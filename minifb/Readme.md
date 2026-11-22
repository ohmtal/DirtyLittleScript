# planing minifb binding for DLS or better 13 ?? .. not sure .. 

- Small memory footprint: `cmake -B build -DMINIFB_USE_OPENGL_API=OFF -DMINIFB_USE_WAYLAND_API=OFF`
- basic 
    - `mfb::new WITH HEIGHT [WINDOWOPTION]`
    - `mfb::pixel obj x y color`
    -  Events need to look first how minifb handle the events so i add it like in 
       BaseFlux or SDL3 Demo
    
- port PixelBuffer from 13 to DLS for painting directly to buffer 
