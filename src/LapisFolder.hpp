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

		const RunType type() const override;
		std::optional<std::filesystem::path> tileLayoutVector() const override;
		size_t nTiles() const override;
		size_t nTilesWithData() const;
		std::string tileNameFromTile(size_t index) const;
		std::string tileNameFromTile(lapis::rowcol_t row, lapis::rowcol_t col) const;
		bool shouldTileExist(size_t n) const;
		bool shouldTileExist(lapis::rowcol_t row, lapis::rowcol_t col) const;

		const lapis::CoordRef crs() const override;
		std::optional<lapis::LinearUnit> units() const override;
		std::optional<lapis::Alignment> metricAlignment() const override;
		std::optional<lapis::Alignment> csmAlignment() const override;
		lapis::Alignment layoutAlignment() const;

		const lapis::Extent& extent() const;
		std::optional<lapis::Extent> extentByTile(size_t index) const override;
		std::optional<lapis::Extent> extentByTile(lapis::rowcol_t row, lapis::rowcol_t col) const;

		lapis::VectorDataset<lapis::Point> allHighPoints() const override;
		std::optional<std::filesystem::path> highPoints(size_t index) const override;
		std::optional<std::filesystem::path> highPoints(lapis::rowcol_t row, lapis::rowcol_t col) const;
		lapis::VectorDataset<lapis::Point> highPoints(const lapis::Extent& e) const;

		std::optional<std::filesystem::path> mcGaugheyPolygons(size_t index) const;
		std::optional<std::filesystem::path> mcGaugheyPolygons(lapis::rowcol_t row, lapis::rowcol_t col) const;
		lapis::VectorDataset<lapis::MultiPolygon> mcGaugheyPolygons(const lapis::Extent& e) const;

		lapis::VectorDataset<lapis::MultiPolygon> allPolygons() const override;
		lapis::VectorDataset<lapis::MultiPolygon> polygons(const lapis::Extent& e) const override;
		std::optional<std::filesystem::path> polygons(size_t index) const override;

		std::optional<std::filesystem::path> watershedSegmentRaster(size_t index) const override;
		std::optional<std::filesystem::path> watershedSegmentRaster(lapis::rowcol_t row, lapis::rowcol_t col) const;
		std::optional<lapis::Raster<lapis::taoid_t>> watershedSegmentRaster(const::lapis::Extent& e) const;

		std::optional<std::filesystem::path> intensityRaster(size_t index) const override;
		std::optional<std::filesystem::path> intensityRaster(lapis::rowcol_t row, lapis::rowcol_t col) const;
		std::optional<lapis::Raster<lapis::intensity_t>> intensityRaster(const lapis::Extent& e) const override;

		std::optional<std::filesystem::path> maxHeightRaster(size_t index) const override;
		std::optional<std::filesystem::path> maxHeightRaster(lapis::rowcol_t row, lapis::rowcol_t col) const;
		std::optional<lapis::Raster<lapis::csm_t>> maxHeightRaster(const lapis::Extent& e) const override;

		std::optional<std::filesystem::path> csmRaster(size_t index) const override;
		std::optional<std::filesystem::path> csmRaster(lapis::rowcol_t row, lapis::rowcol_t col) const;
		std::optional<lapis::Raster<lapis::csm_t>> csmRaster(const lapis::Extent& e) const override;

		std::function<lapis::CoordXY(const lapis::ConstFeature<lapis::MultiPolygon>&)> coordGetter() const override;
		std::function<lapis::coord_t(const lapis::ConstFeature<lapis::MultiPolygon>&)> heightGetter() const override;
		std::function<lapis::coord_t(const lapis::ConstFeature<lapis::MultiPolygon>&)> radiusGetter() const override;
		std::function<lapis::coord_t(const lapis::ConstFeature<lapis::MultiPolygon>&)> areaGetter() const override;
	private:
		std::filesystem::path _folder;
		lapis::Raster<bool> _layoutRaster;
		std::string _name;

		std::optional<std::filesystem::path> _getMetricByName(const std::string& baseName, bool allReturns = true) const;
	};

	//this checks for two things: the presence of TileLayout.shp, and the presence of FullParameters.ini
	bool isLapisFolder(const std::filesystem::path& path);
	std::optional<LapisFolder> getRunIfLapis(const std::filesystem::path& path);
}

#endif