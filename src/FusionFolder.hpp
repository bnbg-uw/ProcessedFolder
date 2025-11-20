#pragma once
#ifndef FUSIONFOLDER_H
#define FUSIONFOLDER_H

#include "ProcessedFolder.hpp"

namespace processedfolder {
	//Repairs an issue that emerges in certain fusion runs where the resolution (and by extension, the origin and extent) get slightly messed up
	//You specify an expected resolution and origin and a tolerance. If the actual resolution is farther from the expected than the tolerance, the raster is considered unrepairable and an exception is thrown
	//Otherwise, the resolution is set to be the expected resolution and the extent is snapped to the grid inferred from the origin and resolution
	template<class T>
	lapis::Raster<T> repairResolution(const lapis::Raster<T>& r, lapis::coord_t expectedXRes, lapis::coord_t expectedYRes, lapis::coord_t expectedXOrigin, lapis::coord_t expectedYOrigin, double tolerance = 0.1) {
		if (r.xres() == expectedXRes && r.yres() == expectedYRes) {
			return(r);
		}
		
		auto xmin = r.xmin();
		auto ymin = r.ymin();
		auto xres = r.xres();
		auto yres = r.yres();
		if (r.xres() != expectedXRes) {
			if (std::abs(r.xres() - expectedXRes) / expectedXRes < tolerance) {
				xmin = std::round((r.xmin() - expectedXOrigin) / expectedXRes) * expectedXRes;
				xres = expectedXRes;
			}
			else {
				throw lapis::AlignmentMismatchException("Raster not repairable");
			}
		}
		if (r.yres() != expectedYRes) {
			if (std::abs(r.yres() - expectedYRes) / expectedYRes < tolerance) {
				ymin = std::round((r.ymin() - expectedYOrigin) / expectedYRes) * expectedYRes;
				yres = expectedYRes;
			}
			else {
				throw lapis::AlignmentMismatchException("Raster not repairable");
			}
		}
		
		lapis::Alignment a{ xmin, ymin, r.nrow(), r.ncol(), xres, yres, r.crs() };
		lapis::Raster<T> out{ a };
		for (lapis::cell_t c = 0; c < out.ncell(); ++c) {
			out[c].value() = r[c].value();
			out[c].has_value() = r[c].has_value();
		}
		return(out);
	}

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