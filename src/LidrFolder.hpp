#pragma once
#ifndef LIDRFOLDER_H
#define LIDRFOLDER_H

#include "ProcessedFolder.hpp"


namespace processedfolder {
	//This is just for rxgaming so it doesn't support most standard gridmetrics style layers
	class LidRFolder : public ProcessedFolder {
	public:

		LidRFolder(const std::filesystem::path& folder);

		const std::filesystem::path dir() const override;

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

		const RunType type() const override;
		std::optional<std::filesystem::path> tileLayoutVector() const override;
		size_t nTiles() const override;
		const lapis::CoordRef crs() const override;
		const lapis::Extent& extent() const override;
		std::optional<lapis::LinearUnit> units() const override;
		std::optional<lapis::Alignment> metricAlignment() const override;
		std::optional<lapis::Alignment> csmAlignment() const override;

		std::optional<lapis::Extent> extentByTile(size_t index) const override;

		lapis::VectorDataset<lapis::Point> allHighPoints() const override;
		std::optional<std::filesystem::path> highPoints(size_t index) const override;
		lapis::VectorDataset<lapis::Point> highPoints(const lapis::Extent& e) const override;

		lapis::VectorDataset<lapis::MultiPolygon> allPolygons() const override;
		lapis::VectorDataset<lapis::MultiPolygon> polygons(const lapis::Extent& e) const override;
		std::optional<std::filesystem::path> polygons(size_t index) const override;

		std::optional<std::filesystem::path> topsRaster(size_t index) const;
		std::optional<lapis::Raster<uint8_t>> topsRaster(const lapis::Extent& e) const;

		std::optional<std::filesystem::path> watershedSegmentRaster(size_t index) const override;
		std::optional<lapis::Raster<lapis::taoid_t>> watershedSegmentRaster(const lapis::Extent& e) const override;

		std::optional<std::filesystem::path> intensityRaster(size_t index) const override;
		std::optional<lapis::Raster<lapis::intensity_t>> intensityRaster(const lapis::Extent& e) const override;

		std::optional<std::filesystem::path> maxHeightRaster(size_t index) const override;
		std::optional<lapis::Raster<lapis::csm_t>> maxHeightRaster(const lapis::Extent& e) const override;

		std::optional<std::filesystem::path> csmRaster(size_t index) const override;
		std::optional<lapis::Raster<lapis::csm_t>> csmRaster(const lapis::Extent& e) const override;

		std::function<lapis::CoordXY(const lapis::ConstFeature<lapis::Point>&)> coordGetter() const override;
		std::function<lapis::coord_t(const lapis::ConstFeature<lapis::Point>&)> heightGetter() const override;
		std::function<lapis::coord_t(const lapis::ConstFeature<lapis::Point>&)> radiusGetter() const override;
		std::function<lapis::coord_t(const lapis::ConstFeature<lapis::Point>&)> areaGetter() const override;
	private:
		std::filesystem::path _folder;
		lapis::VectorDataset<lapis::MultiPolygon> _layout;
		lapis::CoordRef _proj;
		std::string _name;
		std::string _units;
	};

}

#endif