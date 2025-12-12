#pragma once
#ifndef PROCESSEDFOLDER_H
#define PROCESSEDFOLDER_H

#include "ProcessedFolder_pch.hpp"
#include "Raster.hpp"

namespace processedfolder {
	
	class FileNotFoundException : public std::runtime_error {
	public:
		FileNotFoundException() : std::runtime_error("") {}
		FileNotFoundException(std::string s) : std::runtime_error(s) {}
	};

	static std::string stringOrThrow(std::optional<std::filesystem::path> check) {
		if (check.has_value())
			return check.value().string();
		else {
			std::cerr << "Requested file does not exist\n";
			throw FileNotFoundException();
		}
	}

	enum RunType {
		lapis,
		fusion,
		lidr
	};

	class ProcessedFolder {
	public:
		virtual const std::filesystem::path dir() const = 0;

		//these are convenience shortcuts for get metric by name
		virtual std::optional<std::filesystem::path> cover(bool allReturns = true) const = 0;
		virtual std::optional<std::filesystem::path> p95(bool allReturns = true) const = 0;
		virtual std::optional<std::filesystem::path> rumple(bool allReturns = true) const = 0;
		virtual std::optional<std::filesystem::path> p25(bool allReturns = true) const = 0;
		virtual std::optional<std::filesystem::path> meanHeight(bool allReturns = true) const = 0;
		virtual std::optional<std::filesystem::path> stdDevHeight(bool allReturns = true) const = 0;
		virtual std::optional<std::filesystem::path> maskRaster(bool allReturns = true) const = 0;

		//these require the radius for compatibility with FUSION
		virtual std::optional<std::filesystem::path> slope(lapis::coord_t radius, lapis::LinearUnit unit) const = 0;
		virtual std::optional<std::filesystem::path> aspect(lapis::coord_t radius, lapis::LinearUnit unit) const = 0;
		virtual std::optional<std::filesystem::path> tpi(lapis::coord_t radius, lapis::LinearUnit unit) const = 0;
		
		//utility functions
		virtual const RunType type() const = 0;
		virtual std::optional<std::filesystem::path> tileLayoutVector() const = 0;
		virtual size_t nTiles() const = 0;
		virtual const lapis::CoordRef crs() const = 0;
		virtual const lapis::Extent& extent() const = 0;
		virtual std::optional<lapis::LinearUnit> units() const = 0;
		virtual std::optional<lapis::Alignment> metricAlignment() const = 0;
		virtual std::optional<lapis::Alignment> csmAlignment() const = 0;

		virtual std::optional<lapis::Extent> extentByTile(size_t index) const = 0;

		virtual lapis::VectorDataset<lapis::Point> allHighPoints() const = 0;
		virtual lapis::VectorDataset<lapis::Point> highPoints(const lapis::Extent& e) const = 0;
		virtual std::optional<std::filesystem::path> highPoints(size_t index) const = 0;

		virtual lapis::VectorDataset<lapis::MultiPolygon> allPolygons() const = 0;
		virtual lapis::VectorDataset<lapis::MultiPolygon> polygons(const lapis::Extent& e) const = 0;
		virtual std::optional<std::filesystem::path> polygons(size_t index) const = 0;

		virtual std::optional<std::filesystem::path> watershedSegmentRaster(size_t index) const = 0;
		virtual std::optional<lapis::Raster<int>> watershedSegmentRaster(const lapis::Extent& e) const = 0;

		virtual std::optional<std::filesystem::path> intensityRaster(size_t index) const = 0;
		virtual std::optional<lapis::Raster<int>> intensityRaster(const lapis::Extent& e) const = 0;

		virtual std::optional<std::filesystem::path> maxHeightRaster(size_t index) const = 0;
		virtual std::optional<lapis::Raster<double>> maxHeightRaster(const lapis::Extent& e) const = 0;

		virtual std::optional<std::filesystem::path> csmRaster(size_t index) const = 0;
		virtual std::optional<lapis::Raster<double>> csmRaster(const lapis::Extent& e) const = 0;

		virtual lapis::CoordXY coordGetter(const lapis::ConstFeature<lapis::MultiPolygon> ft) const = 0;
		virtual lapis::coord_t heightGetter(const lapis::ConstFeature<lapis::MultiPolygon> ft) const = 0;
		virtual lapis::coord_t radiusGetter(const lapis::ConstFeature<lapis::MultiPolygon> ft) const = 0;
		virtual lapis::coord_t areaGetter(const lapis::ConstFeature<lapis::MultiPolygon> ft) const = 0;

		virtual ~ProcessedFolder() = default;
	};
} //namespace processedfolder

#endif