#pragma once
#ifndef PROCESSEDFOLDER_H
#define PROCESSEDFOLDER_H

#include "ProcessedFolder_pch.hpp"
#include "LapisGis/src/Raster.hpp"

namespace processedfolder {

	static std::string stringOrThrow(std::optional<std::string> check) {
		if (check.has_value())
			return check.value();
		else {
			std::cerr << "Requested file does not exist\n";
			throw FileNotFoundException();
		}
	}

	class FileNotFoundException : public std::runtime_error {
	public:
		FileNotFoundException() : std::runtime_error("") {}
		FileNotFoundException(std::string s) : std::runtime_error(s) {}
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
		virtual std::optional<std::filesystem::path> tileLayoutVector() const = 0;
		virtual int nTiles() const = 0;
		virtual const lapis::CoordRef crs() const = 0;
		virtual const lapis::Extent& extent() const = 0;
		virtual std::optional<lapis::LinearUnit> units() const = 0;
		virtual std::optional<lapis::Alignment> metricAlignment() const = 0;
		virtual std::optional<lapis::Alignment> csmAlignment() const = 0;

		virtual std::optional<lapis::Extent> extentByTile(int index) const = 0;

		virtual lapis::VectorDataset<lapis::Point> allHighPoints() const = 0;
		virtual lapis::VectorDataset<lapis::Point> highPoints(const lapis::Extent& e) const = 0;
		virtual std::optional<std::filesystem::path> highPoints(int index) const = 0;

		virtual std::optional<std::filesystem::path> watershedSegmentRaster(int index) const = 0;
		virtual std::optional<lapis::Raster<int>> watershedSegmentRaster(const lapis::Extent& e) const = 0;

		virtual std::optional<std::filesystem::path> intensityRaster(int index) const = 0;
		virtual std::optional<lapis::Raster<int>> intensityRaster(const lapis::Extent& e) const = 0;

		virtual std::optional<std::filesystem::path> maxHeightRaster(int index) const = 0;
		virtual std::optional<lapis::Raster<double>> maxHeightRaster(const lapis::Extent& e) const = 0;

		virtual std::optional<std::filesystem::path> csmRaster(int index) const = 0;
		virtual std::optional<lapis::Raster<double>> csmRaster(const lapis::Extent& e) const = 0;

		virtual ~ProcessedFolder() = default;
	};
} //namespace processedfolder

#endif