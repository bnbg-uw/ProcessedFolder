#include "FusionFolder.hpp"

namespace processedfolder {
	namespace fs = std::filesystem;

	FusionFolder::FusionFolder(const fs::path& folder)
	{
		if (!fs::is_directory(folder)) {
			throw std::invalid_argument("Folder does not exist");
		}
		const auto layoutregex = std::regex{ "Layout_shapefiles" };
		const auto productsregex = std::regex{ "Products.*" };
		const auto finalregex = std::regex{ "FINAL.*" };

		std::queue<fs::path> candidates;
		candidates.push(folder);

		while (candidates.size()) {
			const fs::path& candidate = candidates.front();

			for (fs::path subdir : fs::directory_iterator(candidate)) {
				if (std::regex_match(subdir.stem().string(), layoutregex)) {
					_folder = candidate;
					const auto ptilesregex = std::regex{ R"(.*_ProcessingTiles\.shp)" };
					for (fs::path file : fs::directory_iterator(_folder / "Layout_shapefiles")) {
						if (std::regex_match(file.filename().string(), ptilesregex)) {
							_layoutPath = file;
							_layout = lapis::VectorDataset<lapis::Polygon>(_layoutPath.string());
							break;
						}
					}
					_proj = lapis::Raster<int>(maskRaster().value().string()).crs();
					_layout.projectInPlace(_proj);
					return;
				}
				if (std::regex_match(subdir.stem().string(), productsregex) ||
					std::regex_match(subdir.stem().string(), finalregex)) {
					candidates.push(subdir);
				}
			}
			candidates.pop();
		}
		throw std::invalid_argument("Not a fusion folder");
	}

	const fs::path FusionFolder::dir() const
	{
		return _folder;
	}

	std::optional<fs::path> FusionFolder::cover(bool allReturns) const
	{
		std::string search;
		if (allReturns) {
			search = "all_cover_aboveUNITS";
		}
		else {
			search = "1st_cover_aboveUNITS";
		}
		return _getMetric(search, "Metrics");
	}

	std::optional<fs::path> FusionFolder::p95(bool allReturns) const
	{
		std::string search;
		if (allReturns) {
			search = "elev_P95_UNITSplus";
		}
		else {
			search = "FIRST_RETURNS_elev_P95_UNITSplus";
		}
		return _getMetric(search, "Metrics");
	}

	std::optional<fs::path> FusionFolder::rumple(bool allReturns) const
	{
		if (!allReturns) {
			std::cout << "First return rumple is not a metric calculated by fusion\n";
			return std::optional<fs::path>();
		}
		return _getMetric("canopy_rumple", "CanopyMetrics");
	}

	std::optional<fs::path> FusionFolder::p25(bool allReturns) const
	{
		std::string search;
		if (allReturns) {
			search = "elev_P25_UNITSplus";
		}
		else {
			search = "FIRST_RETURNS_elev_P25_UNITSplus";
		}
		return _getMetric(search, "Metrics");
	}

	std::optional<fs::path> FusionFolder::meanHeight(bool allReturns) const
	{
		std::string search;
		if (allReturns) {
			search = "elev_ave_UNITSplus";
		}
		else {
			search = "FIRST_RETURNS_elev_ave_UNITSplus";
		}
		return _getMetric(search, "Metrics");
	}

	std::optional<fs::path> FusionFolder::stdDevHeight(bool allReturns) const
	{
		std::string search;
		if (allReturns) {
			search = "elev_stddev_UNITSplus";
		}
		else {
			search = "FIRST_RETURNS_elev_stddev_UNITSplus";
		}
		return _getMetric(search, "Metrics");
	}

	std::optional<fs::path> FusionFolder::maskRaster(bool allReturns) const
	{
		std::string search;
		if (allReturns) {
			search = "all_cnt";
		}
		else {
			search = "FIRST_RETURNS_all_cnt";
		}
		return _getMetric(search, "Metrics");
	}

	std::optional<fs::path> FusionFolder::slope(lapis::coord_t radius, lapis::LinearUnit unit) const
	{
		auto converter = lapis::LinearUnitConverter(unit, units());
		return _getTopoMetric("slope", converter(radius));
	}
	std::optional<fs::path> FusionFolder::aspect(lapis::coord_t radius, lapis::LinearUnit unit) const
	{
		auto converter = lapis::LinearUnitConverter(unit, units());
		return _getTopoMetric("aspect", converter(radius));
	}
	std::optional<fs::path> FusionFolder::tpi(lapis::coord_t radius, lapis::LinearUnit unit) const
	{
		auto converter = lapis::LinearUnitConverter(unit, units());
		return _getTopoMetric("tpi", converter(radius));
	}

	std::optional<fs::path> FusionFolder::tileLayoutVector() const
	{
		if (fs::exists(_layoutPath)) {
			return _layoutPath;
		}
		return std::optional<fs::path>();
	}
	int FusionFolder::nTiles() const
	{
		return _layout.nFeature();
	}
	const lapis::CoordRef FusionFolder::crs() const
	{
		return _proj;
	}
	std::optional<lapis::LinearUnit> FusionFolder::units() const
	{
		return _layout.crs().getXYLinearUnits();
	}

	std::optional<lapis::Alignment> FusionFolder::metricAlignment() const
	{
		auto maskFile = maskRaster();
		if (maskFile.has_value()) {
			return lapis::Alignment(maskFile.value().string());
		}
		return std::optional<lapis::Alignment>();
	}

	std::optional<lapis::Alignment> FusionFolder::csmAlignment() const
	{
		auto ntile = nTiles();
		std::optional<fs::path> file;
		for (int cell = 0; cell < ntile; ++cell) {
			file = csmRaster(cell);
			if (file) {
				break;
			}
		}
		if (!file) {
			return std::optional<lapis::Alignment>();
		}
		lapis::Alignment a{ file.value().string() };
		//correcting for issues where the tif format screws things up
		a.defineCRS(crs());
		a = extendAlignment(a, extent(), lapis::SnapType::out);
		return a;
	}
	std::optional<lapis::Extent> FusionFolder::extentByTile(int index) const
	{
		if (index < 0 || index >= nTiles()) {
			return std::optional<lapis::Extent>();
		}
		return _layout.getFeature(index).getGeometry().boundingBox();
	}

	lapis::VectorDataset<lapis::Point> FusionFolder::allHighPoints() const
	{
		lapis::VectorDataset<lapis::Point> out{};
		int ntile = nTiles();
		std::optional<fs::path> file;
		for (int i = 0; i < ntile; ++i) {
			file = highPoints(i);
			if (file) {
				if (out.nFeature()) {
					out.appendFile(file.value());
				}
				else {
					out = lapis::VectorDataset<lapis::Point>(file.value());
				}
			}
		}
		return out;
	}

	lapis::VectorDataset<lapis::Point> FusionFolder::highPoints(const lapis::Extent& e) const {
		
	}

	std::optional<fs::path> FusionFolder::highPoints(int index) const {
		asfd
	}

	template<class T>
	std::optional<lapis::Raster<T>> fineDataByExtentGeneric(const lapis::Extent& e, const lapis::Raster<bool>& tileLayout, std::function<std::optional<fs::path>(int)> byTile) {
		std::optional<lapis::Raster<T>> out{};

		lapis::Extent projE = lapis::QuadExtent(e, tileLayout.crs()).outerExtent();
		if (!projE.overlaps(tileLayout)) {
			return std::optional<lapis::Raster<T>>{};
		}

		for (auto cell : lapis::CellIterator(tileLayout, projE, lapis::SnapType::out)) {
			std::optional<fs::path> filePath = byTile(cell);
			if (!filePath) {
				continue;
			}
			try {
				if (!out.has_value()) {
					lapis::Alignment a{ filePath.value().string() };
					a.defineCRS(tileLayout.crs());
					a = extendAlignment(a, projE, lapis::SnapType::out);
					a = cropAlignment(a, projE, lapis::SnapType::out);
					out = Raster<T>{ a };
				}
				lapis::Raster<T> tile{ filePath.value().string(), projE, lapis::SnapType::out };
				tile.defineCRS(tileLayout.crs());
				out->overlay(tile, [](T a, T b) {return a; });
			}
			catch (LapisGisException e) {
				continue;
			}
		}
		return out;
	}

	std::optional<fs::path> FusionFolder::watershedSegmentRaster(int index) const
	{
		return _getTileMetric("segments_Basin_Map.img", index);
	}

	std::optional<lapis::Raster<int>> FusionFolder::watershedSegmentRaster(const lapis::Extent& e) const
	{
		asdf
	}

	std::optional<fs::path> FusionFolder::intensityRaster(int index) const
	{
		return _getTileMetric("segments_INT_GE_2m_UNITS.img", index);
	}

	std::optional<lapis::Raster<int>> FusionFolder::intensityRaster(const lapis::Extent& e) const
	{
		asdf
	}

	std::optional<fs::path> FusionFolder::maxHeightRaster(int index) const
	{
		return _getTileMetric("segments_Max_Height_Map.img", index);
	}

	std::optional<lapis::Raster<double>> FusionFolder::maxHeightRaster(const lapis::Extent& e) const
	{
		sad
	}

	std::optional<fs::path> FusionFolder::csmRaster(int index) const
	{
		if (index < 0 || index >= nTiles()) {
			return std::optional<std::string>();
		}
		std::string blockName = "BLOCK" + _layout.getStringField(index, "Blocknum");
		auto getUnitFile = [&](const std::string& unitName)->std::optional<std::string> {
			fs::path fullFile = _folder / ("CanopyHeight_" + unitName) / (blockName + "_CHM_TreeSeg_" + unitName + ".img");
			if (fs::exists(fullFile)) {
				return fullFile.string();
			}
			return std::optional<std::string>();
			};

		std::optional<std::string> out = getUnitFile("2p4606FEET");
		if (!out.has_value()) {
			out = getUnitFile("0p75METERS");
		}

		return out;
	}
	std::optional<lapis::Raster<double>> FusionFolder::csmRaster(const lapis::Extent& e) const {
		ads
	}

	std::optional<fs::path> FusionFolder::_getMetric(const std::string& basename, const std::string& folderBaseName) const
	{

		static std::regex units{ "UNITS" };
		fs::path metersFolder = folderBaseName + "_30METERS";
		std::string metersName = std::regex_replace(basename, units, "2") + "_30METERS.img";
		fs::path fullPathMeters = _folder / metersFolder / metersName;

		if (fs::exists(fullPathMeters)) {
			return fullPathMeters.string();
		}

		fs::path feetFolder = folderBaseName + "_98p424FEET";
		std::string feetName = std::regex_replace(basename, units, "6p5616") + "_98p424FEET.img";
		fs::path fullPathFeet = _folder / feetFolder / feetName;
		if (fs::exists(fullPathFeet)) {
			return fullPathFeet.string();
		}

		return std::optional<fs::path>();
	}

	std::optional<fs::path> FusionFolder::_getTopoMetric(const std::string& basename, lapis::coord_t radiusMeters) const
	{
		std::string folderBaseName = "TopoMetrics";
		auto lookForFile = [&](const std::string& resName, lapis::coord_t convFactor)->std::optional<std::string> {
			std::string folderName = folderBaseName + resName;
			if (!fs::exists(_folder / folderName)) {
				return std::optional<std::string>();
			}
			std::regex pattern{ "^topo_" + basename + "_[0-9]*p?[0-9]*(M|F)_[0-9].*" };
			for (fs::path subfile : fs::directory_iterator(_folder / folderName)) {
				std::string filestr = subfile.filename().string();
				if (std::regex_match(filestr, pattern)) {
					std::regex  scalepattern{ "[0-9]*(p?)[0-9]*(M|F)_" };
					std::smatch m;
					std::regex_search(filestr, m, scalepattern);
					std::string scalestring = m.str().substr(0, m.str().size() - 2);
					scalestring = std::regex_replace(scalestring, std::regex{ "p" }, ".");
					auto scalenum = std::stod(scalestring);
					if (std::abs(scalenum - radiusMeters / convFactor) > 1) {
						continue;
					}
					return (_folder / folderName / filestr).string();
				}
			}
			return std::optional<std::string>();
			};

		std::optional<std::string> out = lookForFile("_30METERS", 1);
		if (out.has_value()) {
			return out;
		}
		return lookForFile("_98p424FEET", 0.3048);
	}

	std::optional<fs::path> FusionFolder::_getTileMetric(const std::string& basename, int index) const
	{
		if (index < 0 || index >= nTiles()) {
			return std::optional<fs::path>();
		}

		std::string tileName = _layout.getStringField(index, "Identifier");

		auto getUnitFile = [&](const std::string& unitName)->std::optional<std::string> {
			fs::path segmentsDir = _folder / ("Segments_" + unitName);
			std::regex unitReplace{ "UNITS" };
			std::string fileName = tileName + "_" + std::regex_replace(basename, unitReplace, unitName);
			fs::path fullFile = segmentsDir / fileName;
			if (fs::exists(fullFile)) {
				return fullFile.string();
			}
			return std::optional<std::string>();
			};

		std::optional<std::string> out = getUnitFile("2p4606FEET");
		if (!out.has_value()) {
			out = getUnitFile("0p75METERS");
		}
		return out;
	}
}