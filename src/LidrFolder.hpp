#pragma once
#ifndef LIDRFOLDER_H
#define LIDRFOLDER_H

#include "ProcessedFolder.hpp"

namespace processedfolder {
	//This is just for rxgaming so it doesn't support most standard gridmetrics style layers
	class LidRFolder : public ProcessedFolder {
	public:

		LidRFolder(const std::filesystem::path& folder);

		std::filesystem::path getDir() const override;

		std::optional<std::filesystem::path> cover(bool allReturns = true) const override;
		std::optional<std::filesystem::path> p95(bool allReturns = true) const override;
		std::optional<std::filesystem::path> rumple(bool allReturns = true) const override;
		std::optional<std::filesystem::path> p25(bool allReturns = true) const override;
		std::optional<std::filesystem::path> meanHeight(bool allReturns = true) const override;
		std::optional<std::filesystem::path> stdDevHeight(bool allReturns = true) const override;
		std::optional<std::filesystem::path> maskRaster(bool allReturns = true) const override;

		std::optional<std::filesystem::path> slope(lapis::coord_t radius, lapis::LinearUnit unit) const override;
		std::optional<std::filesystem::path> aspect(lapis::coord_t radius, lapis::LinearUnit unit) const override;
		std::optional<std::filesystem::path> tpi(lapis::coord_t radius, lapis::LinearUnit unit) const override;

		std::optional<std::filesystem::path> tileLayoutVector() const override;
		int nTiles() const override;
		const lapis::CoordRef crs() const override;
		std::optional<lapis::LinearUnit> units() const override;
		std::optional<lapis::Alignment> metricAlignment() const override;
		std::optional<lapis::Alignment> csmAlignment() const override;


		std::optional<lapis::Extent> extentByTile(int index) const override;

		lapis::VectorDataset<lapis::Point> allHighPoints() const override;
		std::optional<std::filesystem::path> highPoints(int i) const override;
		lapis::VectorDataset<lapis::Point> highPoints(const lapis::Extent& e) const override;

		std::optional<std::filesystem::path> getTopsRaster(int index) const;
		std::optional<lapis::Raster<int>> getTopsRaster(const::lapis::Extent& e) const;

		std::optional<std::filesystem::path> watershedSegmentRaster(int index) const override;
		std::optional<lapis::Raster<int>> watershedSegmentRaster(const::lapis::Extent& e) const override;

		std::optional<std::filesystem::path> intensityRaster(int index) const override;
		std::optional<lapis::Raster<int>> intensityRaster(const lapis::Extent& e) const override;

		std::optional<std::filesystem::path> maxHeightRaster(int index) const override;
		std::optional<lapis::Raster<double>> maxHeightRaster(const lapis::Extent& e) const override;

		std::optional<std::filesystem::path> csmRaster(int index) const override;
		std::optional<lapis::Raster<double>> csmRaster(const lapis::Extent& e) const override;

	private:
		std::filesystem::path _folder;
		lapis::VectorDataset<lapis::Polygon> _layout;
		lapis::CoordRef _proj;
		std::string _name;
		std::string _units;

		std::optional<std::string> _getPointMetric(const std::string& baseName, const std::string& units) const;
	};

}

#endif