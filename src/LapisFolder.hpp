#pragma once
#ifndef LAPISFOLDER_H
#define LAPISFOLDER_H

#include "ProcessedFolder.hpp"
#include "boost/program_options.hpp"

namespace processedfolder {
	class LapisFolder : public ProcessedFolder {
	public:

		LapisFolder(const std::filesystem::path& folder);

		const std::filesystem::path dir() const override;
		const std::string& name() const;

		std::optional<std::filesystem::path> cover(bool allReturns = true) const override;
		std::optional<std::filesystem::path> p95(bool allReturns = true) const override;
		std::optional<std::filesystem::path> rumple(bool allReturns = true) const override;
		std::optional<std::filesystem::path> p25(bool allReturns = true) const override;
		std::optional<std::filesystem::path> meanHeight(bool allReturns = true) const override;
		std::optional<std::filesystem::path> stdDevHeight(bool allReturns = true) const override;
		std::optional<std::filesystem::path> maskRaster(bool allReturns = true) const override;
		std::optional<std::filesystem::path> heightPercentile(int percentile, bool allReturns = true) const;

		//defaults because lapis doesn't have radius for slope and aspect
		std::optional<std::filesystem::path> slope(lapis::coord_t radius = 0, lapis::LinearUnit unit = lapis::linearUnitPresets::meter) const override;
		std::optional<std::filesystem::path> aspect(lapis::coord_t radius = 0, lapis::LinearUnit unit = lapis::linearUnitPresets::meter) const override;
		std::optional<std::filesystem::path> tpi(lapis::coord_t radius, lapis::LinearUnit unit) const override;

		std::optional<std::filesystem::path> tileLayoutVector() const override;
		int nTiles() const override;
		int nTilesWithData() const;
		std::string tileNameFromTile(int index) const;
		std::string tileNameFromTile(int row, int col) const;
		bool shouldTileExist(int n) const;
		bool shouldTileExist(int row, int col) const;

		const lapis::CoordRef crs() const override;
		std::optional<lapis::LinearUnit> units() const override;
		std::optional<lapis::Alignment> metricAlignment() const override;
		std::optional<lapis::Alignment> csmAlignment() const override;
		lapis::Alignment layoutAlignment() const;

		const lapis::Extent& extent() const;
		std::optional<lapis::Extent> extentByTile(int index) const override;
		std::optional<lapis::Extent> extentByTile(int row, int col) const;

		lapis::VectorDataset<lapis::Point> allHighPoints() const override;
		std::optional<std::filesystem::path> highPoints(int index) const override;
		std::optional<std::filesystem::path> highPoints(int row, int col) const;
		lapis::VectorDataset<lapis::Point> highPoints(const lapis::Extent& e) const;

		std::optional<std::filesystem::path> mcGaugheyPolygons(int index) const;
		std::optional<std::filesystem::path> mcGaugheyPolygons(int row, int col) const;

		std::optional<std::filesystem::path> watershedSegmentRaster(int index) const override;
		std::optional<std::filesystem::path> watershedSegmentRaster(int row, int col) const;
		std::optional<lapis::Raster<int>> watershedSegmentRaster(const::lapis::Extent& e) const;

		std::optional<std::filesystem::path> intensityRaster(int index) const override;
		std::optional<std::filesystem::path> intensityRaster(int row, int col) const;
		std::optional<lapis::Raster<int>> intensityRaster(const lapis::Extent& e) const override;

		std::optional<std::filesystem::path> maxHeightRaster(int index) const override;
		std::optional<std::filesystem::path> maxHeightRaster(int row, int col) const;
		std::optional<lapis::Raster<double>> maxHeightRaster(const lapis::Extent& e) const override;

		std::optional<std::filesystem::path> csmRaster(int index) const override;
		std::optional<std::filesystem::path> csmRaster(int row, int col) const;
		std::optional<lapis::Raster<double>> csmRaster(const lapis::Extent& e) const override;

	private:
		std::filesystem::path _folder;
		lapis::Raster<bool> _layoutRaster;
		lapis::VectorDataset<lapis::Polygon> _layout;
		std::string _name;

		std::optional<std::filesystem::path> _getMetricByName(const std::string& baseName, bool allReturns = true) const;
	};

	//this checks for two things: the presence of TileLayout.shp, and the presence of FullParameters.ini
	bool isLapisFolder(const std::filesystem::path& path);
	std::optional<LapisFolder> getRunIfLapis(const std::filesystem::path& path);
}

#endif