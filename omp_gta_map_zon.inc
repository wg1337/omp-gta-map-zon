#if defined _omp_gta_map_zon_included
    #endinput
#endif
#define _omp_gta_map_zon_included

#define GTA_MAP_ZON_FILE "map.zon"

#define GTA_MAP_ZON_MAX_ZONES (32)
#define GTA_MAP_ZON_NAME_LENGTH (16)
#define GTA_MAP_ZON_PARENT_LENGTH (16)
#define GTA_MAP_ZON_MAX_LINE_LENGTH (256)

enum E_GTA_MAPZON_LEVEL {
    GTA_MAPZON_LEVEL_DEFAULT = 0,
    GTA_MAPZON_LEVEL_LOS_SANTOS,
    GTA_MAPZON_LEVEL_SAN_FIERRO,
    GTA_MAPZON_LEVEL_LAS_VENTURAS
};

enum E_GTA_MAP_ZONE{
    GTA_MZ_NAME[GTA_MAP_ZON_NAME_LENGTH],
    GTA_MZ_TYPE,
    Float:GTA_MZ_MIN_X,
    Float:GTA_MZ_MIN_Y,
    Float:GTA_MZ_MIN_Z,
    Float:GTA_MZ_MAX_X,
    Float:GTA_MZ_MAX_Y,
    Float:GTA_MZ_MAX_Z,
    E_GTA_MAPZON_LEVEL:GTA_MZ_LEVEL,
    GTA_MZ_PARENT[GTA_MAP_ZON_PARENT_LENGTH]
};

static gMapZonZones[GTA_MAP_ZON_MAX_ZONES][E_GTA_MAP_ZONE];
static gMapZonZoneCount;
static bool:gMapZonLoaded;

static stock MapZon_Reset() {
    gMapZonLoaded = false;
    gMapZonZoneCount = 0;
}

static stock bool:MapZon_IsLineEmpty(const line[]) {
    for(new i = 0; line[i] != '\0'; i++) {
        if(line[i] != ' ' && line[i] != '\t' && line[i] != '\r' && line[i] != '\n') {
            return false;
        }
    }

    return true;
}

static stock bool:MapZon_IsCommentLine(const line[]) {
    new i;

    while(line[i] == ' ' || line[i] == '\t')
        i++;

    return line[i] == '#';
}

static stock bool:MapZon_IsSectionMarker(const line[]) {
    new start;

    while(line[start] == ' ' || line[start] == '\t')
        start++;

    new end = start;

    while(line[end] != '\0' && line[end] != '\r' && line[end] != '\n' && line[end] != ' ' && line[end] != '\t') {
        end++;
    }

    new length = end - start;

    if(length == 4) {
        if(tolower(line[start])     == 'z' && tolower(line[start + 1]) == 'o' && tolower(line[start + 2]) == 'n' && tolower(line[start + 3]) == 'e') {
            return true;
        }
    }

    if(length == 3) {
        if(tolower(line[start])     == 'e' && tolower(line[start + 1]) == 'n' && tolower(line[start + 2]) == 'd') {
            return true;
        }
    }

    return false;
}

static stock bool:MapZon_CopyTrimmedRange(const source[], start, end, destination[], destinationSize) {
    while(start < end && (source[start] == ' ' || source[start] == '\t'))
        start++;

    while(end > start && ( source[end - 1] == ' ' || source[end - 1] == '\t' || source[end - 1] == '\r' || source[end - 1] == '\n')) {
        end--;
    }

    new length = end - start;

    if(length <= 0 || length >= destinationSize)
        return false;

    for(new i = 0; i < length; i++)
        destination[i] = source[start + i];

    destination[length] = '\0';
    return true;
}

static stock bool:MapZon_ParseInteger(const value[], &result) {
    new i;
    new sign = 1;

    if(value[0] == '-') {
        sign = -1;
        i++;
    }
    else if(value[0] == '+') {
        i++;
    }

    if(value[i] == '\0')
        return false;

    result = 0;

    for(; value[i] != '\0'; i++) {
        if(value[i] < '0' || value[i] > '9')
            return false;

        result = (result * 10) + (value[i] - '0');
    }

    result *= sign;
    return true;
}

static stock bool:MapZon_ParseFloat(const value[], &Float:result) {
    new i;
    new sign = 1;

    if(value[0] == '-') {
        sign = -1;
        i++;
    }
    else if(value[0] == '+') {
        i++;
    }

    if(value[i] == '\0')
        return false;

    new bool:hasDigit;
    new bool:hasDecimal;
    new Float:number;
    new Float:fraction = 0.1;

    for(; value[i] != '\0'; i++) {
        if(value[i] == '.') {
            if(hasDecimal)
                return false;

            hasDecimal = true;
            continue;
        }

        if(value[i] < '0' || value[i] > '9')
            return false;

        hasDigit = true;

        if(hasDecimal) {
            number += float(value[i] - '0') * fraction;
            fraction *= 0.1;
        }
        else {
            number = number * 10.0 + float(value[i] - '0');
        }
    }

    if(!hasDigit)
        return false;

    result = number * float(sign);
    return true;
}

static stock bool:MapZon_ParseLine(const line[], zone) {
    if(zone < 0 || zone >= GTA_MAP_ZON_MAX_ZONES)
        return false;

    new fields[10][32];
    new field;
    new fieldStart;
    new length = strlen(line);

    for(new i = 0; i <= length; i++) {
        if(line[i] == ',' || line[i] == '\0' || line[i] == '\r' || line[i] == '\n') {
            if(field >= sizeof(fields))
                return false;

            if(!MapZon_CopyTrimmedRange(line, fieldStart, i, fields[field], sizeof(fields[]))) {
                return false;
            }

            field++;
            fieldStart = i + 1;

            if(line[i] != ',')
                break;
        }
    }

    if(field != sizeof(fields))
        return false;

    if(strlen(fields[0]) >= GTA_MAP_ZON_NAME_LENGTH)
        return false;

    if(strlen(fields[9]) >= GTA_MAP_ZON_PARENT_LENGTH)
        return false;

    new zoneType;
    new level;

    new Float:minX;
    new Float:minY;
    new Float:minZ;

    new Float:maxX;
    new Float:maxY;
    new Float:maxZ;

    if(!MapZon_ParseInteger(fields[1], zoneType))
        return false;

    if(!MapZon_ParseFloat(fields[2], minX))
        return false;

    if(!MapZon_ParseFloat(fields[3], minY))
        return false;

    if(!MapZon_ParseFloat(fields[4], minZ))
        return false;

    if(!MapZon_ParseFloat(fields[5], maxX))
        return false;

    if(!MapZon_ParseFloat(fields[6], maxY))
        return false;

    if(!MapZon_ParseFloat(fields[7], maxZ))
        return false;

    if(!MapZon_ParseInteger(fields[8], level))
        return false;

    if(level < _:GTA_MAPZON_LEVEL_DEFAULT || level > _:GTA_MAPZON_LEVEL_LAS_VENTURAS) {
        return false;
    }

    format(gMapZonZones[zone][GTA_MZ_NAME], GTA_MAP_ZON_NAME_LENGTH, "%s", fields[0]);

    gMapZonZones[zone][GTA_MZ_TYPE] = zoneType;

    gMapZonZones[zone][GTA_MZ_MIN_X] = minX;
    gMapZonZones[zone][GTA_MZ_MIN_Y] = minY;
    gMapZonZones[zone][GTA_MZ_MIN_Z] = minZ;

    gMapZonZones[zone][GTA_MZ_MAX_X] = maxX;
    gMapZonZones[zone][GTA_MZ_MAX_Y] = maxY;
    gMapZonZones[zone][GTA_MZ_MAX_Z] = maxZ;

    gMapZonZones[zone][GTA_MZ_LEVEL] = E_GTA_MAPZON_LEVEL:level;

    format(gMapZonZones[zone][GTA_MZ_PARENT], GTA_MAP_ZON_PARENT_LENGTH, "%s", fields[9]);

    return true;
}

/*
* --------------------------------------------------------------------------
* USABLE FUNCTIONS
* --------------------------------------------------------------------------
*/

stock bool:LoadMapZon(const filename[] = GTA_MAP_ZON_FILE) {
    MapZon_Reset();

    new File:file = fopen(filename, io_read);

    if(!file) {
        printf("[omp-gta-map-zon] ERROR: Failed to open %s", filename);
        return false;
    }

    new line[GTA_MAP_ZON_MAX_LINE_LENGTH];

    while(fread(file, line)) {
        new length = strlen(line);

        if(length >= GTA_MAP_ZON_MAX_LINE_LENGTH - 1 && line[length - 1] != '\n') {
            printf("[omp-gta-map-zon] ERROR: Line exceeds maximum length of %d characters.", GTA_MAP_ZON_MAX_LINE_LENGTH - 1);

            fclose(file);
            MapZon_Reset();
            return false;
        }

        if(MapZon_IsLineEmpty(line) || MapZon_IsCommentLine(line) || MapZon_IsSectionMarker(line)) {
            continue;
        }

        if(gMapZonZoneCount >= GTA_MAP_ZON_MAX_ZONES) {
            printf("[omp-gta-map-zon] ERROR: Maximum zone count of %d exceeded.", GTA_MAP_ZON_MAX_ZONES);

            fclose(file);
            MapZon_Reset();
            return false;
        }

        if(!MapZon_ParseLine(line, gMapZonZoneCount)) {
            printf(
                "[omp-gta-map-zon] ERROR: Failed to parse zone %d: %s",
                gMapZonZoneCount,
                line
            );

            fclose(file);
            MapZon_Reset();
            return false;
        }

        gMapZonZoneCount++;
    }

    fclose(file);

    if(gMapZonZoneCount <= 0) {
        print("[omp-gta-map-zon] ERROR: No map zones were loaded.");
        MapZon_Reset();
        return false;
    }

    gMapZonLoaded = true;

    printf("[omp-gta-map-zon] Loaded %d map zones.", gMapZonZoneCount);

    return true;
}

stock bool:IsMapZonLoaded() {
    return gMapZonLoaded;
}

stock GetMapZonZoneCount() {
    if(!IsMapZonLoaded())
        return 0;

    return gMapZonZoneCount;
}

/*
* Returns the map.zon zone index at a certain location.
*/
stock GetMapZonIndexAt(Float:x, Float:y, Float:z) {
    if(!IsMapZonLoaded())
        return -1;

    for(new i = 0; i < gMapZonZoneCount; i++) {
        if(
            x >= gMapZonZones[i][GTA_MZ_MIN_X] && 
            x <= gMapZonZones[i][GTA_MZ_MAX_X] && 
            y >= gMapZonZones[i][GTA_MZ_MIN_Y] && 
            y <= gMapZonZones[i][GTA_MZ_MAX_Y] && 
            z >= gMapZonZones[i][GTA_MZ_MIN_Z] && 
            z <= gMapZonZones[i][GTA_MZ_MAX_Z]) {

            return i;
        }
    }

    return -1;
}

/*
* Returns the map.zon level at a certain location.
*/
stock E_GTA_MAPZON_LEVEL:GetMapZonLevelAt(Float:x, Float:y, Float:z) {
    new zone = GetMapZonIndexAt(x, y, z);

    if(zone == -1)
        return GTA_MAPZON_LEVEL_DEFAULT;

    return gMapZonZones[zone][GTA_MZ_LEVEL];
}

/*
* Returns the map.zon zone name, for example "SF02".
*/
stock bool:GetMapZonName(zone, output[], size = sizeof(output)) {
    output[0] = '\0';

    if(!IsMapZonLoaded() || zone < 0 || zone >= gMapZonZoneCount) {
        return false;
    }

    format(output, size, "%s", gMapZonZones[zone][GTA_MZ_NAME]);

    return true;
}


/*
* Tests loaded map.zon
* TestMapZon(); - Tests all loaded zones.
* TestMapZon(0); - Tests one specific zone.
*/
stock bool:TestMapZon(filterZone = -1) {
    if(!IsMapZonLoaded()) {
        print("[omp-gta-map-zon] TEST ERROR: map.zon is not loaded.");
        return false;
    }

    if(filterZone < -1 || filterZone >= gMapZonZoneCount) {
        printf("[omp-gta-map-zon] TEST ERROR: Invalid zone index %d.", filterZone);
        return false;
    }

    new testedZones;
    new invalidZones;

    for(new zone = 0; zone < gMapZonZoneCount; zone++) {
        if(filterZone != -1 && zone != filterZone)
            continue;

        testedZones++;

        printf("[omp-gta-map-zon] [%d] %s type=%d level=%d parent=%s", zone, gMapZonZones[zone][GTA_MZ_NAME], gMapZonZones[zone][GTA_MZ_TYPE], _:gMapZonZones[zone][GTA_MZ_LEVEL], gMapZonZones[zone][GTA_MZ_PARENT]);
        printf("[omp-gta-map-zon]      min=(%.2f, %.2f, %.2f) max=(%.2f, %.2f, %.2f)", gMapZonZones[zone][GTA_MZ_MIN_X], gMapZonZones[zone][GTA_MZ_MIN_Y], gMapZonZones[zone][GTA_MZ_MIN_Z], gMapZonZones[zone][GTA_MZ_MAX_X], gMapZonZones[zone][GTA_MZ_MAX_Y], gMapZonZones[zone][GTA_MZ_MAX_Z]);

        if(gMapZonZones[zone][GTA_MZ_MIN_X] > gMapZonZones[zone][GTA_MZ_MAX_X] || gMapZonZones[zone][GTA_MZ_MIN_Y] > gMapZonZones[zone][GTA_MZ_MAX_Y] || gMapZonZones[zone][GTA_MZ_MIN_Z] > gMapZonZones[zone][GTA_MZ_MAX_Z]) {
            print("[omp-gta-map-zon]      ERROR: Invalid bounds.");
            invalidZones++;
        }
    }

    printf("[omp-gta-map-zon] TEST: tested=%d invalid=%d", testedZones, invalidZones);

    if(invalidZones != 0) {
        print("[omp-gta-map-zon] TEST FAILED.");
        return false;
    }

    print("[omp-gta-map-zon] TEST PASSED.");
    return true;
}
