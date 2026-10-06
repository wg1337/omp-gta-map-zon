# What is it?

This is a simple include to parse GTA:SA "map.zon" file. This file contains information about where each map zone starts and ends. These map zones are used in GTA:sA to determine ped variants, for example, where do SF cops spawn and where to country cops spawn.

# How to use it?

1) Place "map.zon" in your "scriptfiles/" folder

2) Include it in your script:
```
#include <omp_gta_map_zon>

public OnFilterScriptInit() {
    ....
    if(!LoadMapZon()) {
        print("ERROR: Failed to load map.zon");
        return false;
    }
    ....
    return true;
}
```

3) Use the provided functions, for example:
```
//Get the map zone level at specific coordinates
new E_GTA_MAPZON_LEVEL:level = GetMapZonLevelAt(-2000, 100, 10); //Returns GTA_MAPZON_LEVEL_SAN_FIERRO
```
