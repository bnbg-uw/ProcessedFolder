#pragma once
#ifndef FUSIONFOLDER_H
#define FUSIONFOLDER_H

#include "ProcessedFolder.hpp"

namespace processedfolder {
	class FusionFolder : public ProcessedFolder {
	public:

		FusionFolder(const std::filesystem::path& folder);

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

		std::optional<std::filesystem::path> tileLayoutVector() const override;
		size_t nTiles() const override;
		const lapis::CoordRef crs() const override;
		std::optional<lapis::LinearUnit> units() const override;
		std::optional<lapis::Alignment> metricAlignment() const override;
		std::optional<lapis::Alignment> csmAlignment() const override;

		std::optional<lapis::Extent> extentByTile(size_t index) const override;

		lapis::VectorDataset<lapis::Point> allHighPoints() const override;
		std::optional<std::filesystem::path> highPoints(size_t index) const override;
		lapis::VectorDataset<lapis::Point> highPoints(const lapis::Extent& e) const override;

		std::optional<std::filesystem::path> watershedSegmentRaster(size_t index) const override;
		std::optional<lapis::Raster<int>> watershedSegmentRaster(const lapis::Extent& e) const override;

		std::optional<std::filesystem::path> intensityRaster(size_t index) const override;
		std::optional<lapis::Raster<int>> intensityRaster(const lapis::Extent& e) const override;

		std::optional<std::filesystem::path> maxHeightRaster(size_t index) const override;
		std::optional<lapis::Raster<double>> maxHeightRaster(const lapis::Extent& e) const override;

		std::optional<std::filesystem::path> csmRaster(size_t index) const override;
		std::optional<lapis::Raster<double>> csmRaster(const lapis::Extent& e) const override;

	private:
		std::filesystem::path _folder;
		std::filesystem::path _layoutPath;
		lapis::VectorDataset<lapis::Polygon> _layout;
		lapis::CoordRef _proj;

		std::optional<std::filesystem::path> _getMetric(const std::string& basename, const std::string& folderBaseName) const;
		std::optional<std::filesystem::path> _getTopoMetric(const std::string& basename, lapis::coord_t radiusMeters) const;
		std::optional<std::filesystem::path> _getTileMetric(const std::string& basename, size_t index) const;
	};
}

#endif