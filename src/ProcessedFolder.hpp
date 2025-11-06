#pragma once
#ifndef PROCESSEDFOLDER_H
#define PROCESSEDFOLDER_H

#include<string>
#include<optional>
#include<memory>

#include"LICO/LICO.hpp"

namespace lidar {

	namespace sp = spatial;
	namespace fs = std::filesystem;

	class FileNotFoundException : public std::runtime_error {
	public:
		FileNotFoundException() : std::runtime_error("") {}
		FileNotFoundException(std::string s) : std::runtime_error(s) {}
	};

	using crownFunc = std::function<void(lico::Tao&)>;

	class ProcessedFolder {
	public:
		virtual fs::path getDir() const = 0;

		virtual std::optional<std::string> getCover() const = 0;
		virtual std::optional<std::string> getP95() const = 0;
		virtual std::optional<std::string> getRumple() const = 0;
		virtual std::optional<std::string> getP25() const = 0;
		virtual std::optional<std::string> getMeanHeight() const = 0;
		virtual std::optional<std::string> getStdDevHeight() const = 0;

		virtual std::optional<std::string> getSlope(sp::coord_t radiusMeters) const = 0;
		virtual std::optional<std::string> getAspect(sp::coord_t radiusMeters) const = 0;
		virtual std::optional<std::string> getTPI(sp::coord_t radiusMeters) const = 0;
		
		virtual std::optional<std::string> getMaskRaster() const = 0;
		virtual std::optional<std::string> getTileLayoutVector() const = 0;
		virtual int nTiles() const = 0;
		virtual const sp::CoordRef& getProjection() const = 0;
		virtual sp::coord_t getConvFactor() const = 0;
		virtual std::optional<sp::Alignment> getMetricAlignment() const = 0;
		virtual std::optional<sp::Extent> extentByTile(int index) const = 0;

		virtual lico::TaoList readAllHighPoints(int nThreads, double fixedRadius = 0) const = 0;
		virtual lico::TaoList getHighPointsByTile(int i, double fixedRadius = 0) const = 0;

		virtual std::optional<std::string> getSegmentRaster(int index) const = 0;
		virtual std::optional<std::string> getCanopyIntensityRaster(int index) const = 0;
		virtual std::optional<std::string> getMaxHeightRaster(int index) const = 0;
		virtual std::optional<std::string> getCsmRaster(int index) const = 0;

		virtual ~ProcessedFolder() = default;

	};

	std::unique_ptr<ProcessedFolder> getProcessedFolderReader(const std::string& folder);
	std::string stringOrThrow(std::optional<std::string> check);
}

#endif