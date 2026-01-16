#pragma once
#ifndef PROCESSEDFOLDER_PCH
#define PROCESSEDFOLDER_PCH

#include<string>
#include<optional>
#include<memory>
#include<filesystem>
#include<regex>
#include<queue>

#include<Raster.hpp>
#include<RasterAlgos.hpp>
#include<boost/program_options.hpp>

//these types are defined in Lapis, not LapisGis, so I'm redefining them here
namespace lapis {
    using csm_t = lapis::coord_t;
    using taoid_t = uint32_t; //int64 would be ideal but none of the common raster formats support it
}

#endif