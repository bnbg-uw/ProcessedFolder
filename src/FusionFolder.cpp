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

	const RunType FusionFolder::type() const {
		return RunType::fusion;
	}

	std::optional<fs::path> FusionFolder::tileLayoutVector() const
	{
		if (fs::exists(_layoutPath)) {
			return _layoutPath;
		}
		return std::optional<fs::path>();
	}
	size_t FusionFolder::nTiles() const
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

	std::optional<lapis::Extent> FusionFolder::extentByTile(size_t index) const
	{
		if (index < 0 || index >= nTiles()) {
			return std::optional<lapis::Extent>();
		}
		return _layout.getFeature(index).getGeometry().boundingBox();
	}

	lapis::VectorDataset<lapis::Point> FusionFolder::allHighPoints() const
	{
		return highPoints(_layout.extent());
	}

	lapis::VectorDataset<lapis::Point> FusionFolder::highPoints(const lapis::Extent& e) const {
		lapis::VectorDataset<lapis::Point> out{};
		bool outInit = false;

		lapis::Extent projE = lapis::QuadExtent(e, _layout.crs()).outerExtent();
		if (!projE.overlaps(_layout.extent())) {
			return out;
		}

		for (size_t i = 0; i < _layout.nFeature(); ++i) {
			std::optional<fs::path> filePath = highPoints(i);
			if (filePath) {
				auto tileExtent = extentByTile(i).value();
				if (!projE.overlaps(tileExtent)) {
					continue;
				}
				lapis::VectorDataset<lapis::Point> thisPoints{ filePath.value() };
				if (thisPoints.nFeature()) {
					if (!outInit) {
						out = lapis::emptyVectorDatasetFromTemplate(thisPoints);
					}
					for (lapis::ConstFeature<lapis::Point> ft : thisPoints) {
						if (projE.contains(ft.getGeometry().x(), ft.getGeometry().y())) {
							if (tileExtent.contains(ft.getGeometry().x(), ft.getGeometry().y())) {
								out.addFeature(ft);
							}
						}
					}
				}
			}
		}
		return out;
	}

	std::optional<fs::path> FusionFolder::highPoints(size_t index) const {
		fs::path segments = _folder / "Segments_2p4606FEET";
		if (!fs::exists(segments)) {
			segments = _folder / "Segments_0p75METERS";
		}
		auto identifier = this->_layout.getStringField(index, "Identifier");
		fs::path thissegments = segments / (identifier + "_segments_Polygons.shp");
		if (fs::exists(thissegments)) {
			return thissegments;
		}
		return std::optional<fs::path>();
	}

	lapis::VectorDataset<lapis::MultiPolygon> FusionFolder::allPolygons() const {
		return polygons(_layout.extent());
	}
	
	lapis::VectorDataset<lapis::MultiPolygon> FusionFolder::polygons(const lapis::Extent& e) const {
		lapis::VectorDataset<lapis::MultiPolygon> out;
		std::regex xr{ ".*HighX.*" };
		std::regex yr{ ".*HighY.*" };
		std::regex ar{ ".*Area.*" };
		std::regex hr{ ".*MaxHt.*" };

		lapis::Extent projE = lapis::QuadExtent(e, _layout.crs()).outerExtent();
		if (!projE.overlaps(_layout.extent())) {
			return out;
		}

		for (size_t i = 0; i < nTiles(); ++i) {
			auto polygonFile = polygons(i);
			if (polygonFile) {
				auto tileExtent = extentByTile(i).value();
				if (!projE.overlaps(tileExtent)) {
					continue;
				}
				lapis::VectorDataset<lapis::MultiPolygon> thisPolygons{ polygonFile.value() };
				if (_x == "") {
					for (auto name : thisPolygons.getAllFieldNames()) {
						if (std::regex_match(name, xr)) {
							_x = name;
						}
						else if (std::regex_match(name, yr)) {
							_y = name;
						}
						else if (std::regex_match(name, ar)) {
							_a = name;
						}
						else if (std::regex_match(name, hr)) {
							_h = name;
						}
					}
					if (_x == "" || _y == "" || _a == "" || _h == "") {
						std::cerr << "Found polygon files but could not deduce one of x,y,area, or height from the column names.\n";
						std::cerr << polygonFile.value().string() << "\n";
						throw FileNotFoundException("Found polygon files but could not deduce one of x,y,area, or height from the column names.");
					}
					out = lapis::emptyVectorDatasetFromTemplate(thisPolygons);
				}
				for (lapis::ConstFeature<lapis::MultiPolygon> ft : thisPolygons) {
					auto x = ft.getNumericField<lapis::coord_t>(_x);
					auto y = ft.getNumericField<lapis::coord_t>(_y);
					if (tileExtent.contains(x, y) && projE.contains(x, y)) {
						out.addFeature(ft);
					}
				}
			}
		}
		return out;
	}
	
	std::optional<fs::path> FusionFolder::polygons(size_t index) const {
		auto identifier = this->_layout.getStringField(index, "Identifier");
		fs::path candidate = _folder / "Segments_2p4606FEET" / (identifier + "_segments_Polygons.shp");
		if (fs::exists(candidate)) {
			return candidate;
		}
		candidate = _folder / "Segments_0p75METERS" / (identifier + "_segments_Polygons.shp");

		if (fs::exists(candidate)) {
			return candidate;
		}
		return std::optional<fs::path>();
	}

	template<class T>
	std::optional<lapis::Raster<T>> fineDataByExtentGeneric(const lapis::Extent& e, const lapis::VectorDataset<lapis::Polygon>& tileLayout, std::function<std::optional<fs::path>(size_t)> byTile) {
		std::optional<lapis::Raster<T>> out{};

		lapis::Extent projE = lapis::QuadExtent(e, tileLayout.crs()).outerExtent();
		if (!projE.overlaps(tileLayout.extent())) {
			return std::optional<lapis::Raster<T>>{};
		}

		for (size_t i = 0; i < tileLayout.nFeature(); ++i) {
			std::optional<fs::path> filePath = byTile(i);
			if (!filePath) {
				continue;
			}
			try {
				if (!out.has_value()) {
					lapis::Alignment a{ filePath.value().string() };
					a.defineCRS(tileLayout.crs());
					a = extendAlignment(a, projE, lapis::SnapType::out);
					a = cropAlignment(a, projE, lapis::SnapType::out);
					out = lapis::Raster<T>{ a };
				}
				lapis::Raster<T> tile{ filePath.value().string(), projE, lapis::SnapType::out };
				tile.defineCRS(tileLayout.crs());
				out->overlayInside(tile);
			}
			catch (lapis::LapisGisException e) {
				continue;
			}
		}
		return out;
	}

	std::optional<fs::path> FusionFolder::watershedSegmentRaster(size_t index) const
	{
		return _getTileMetric("segments_Basin_Map.img", index);
	}

	std::optional<lapis::Raster<int>> FusionFolder::watershedSegmentRaster(const lapis::Extent& e) const
	{
		return fineDataByExtentGeneric<int>(e, _layout, [&](size_t n) { return watershedSegmentRaster(n); });
	}

	std::optional<fs::path> FusionFolder::intensityRaster(size_t index) const
	{
		return _getTileMetric("segments_INT_GE_2m_UNITS.img", index);
	}

	std::optional<lapis::Raster<int>> FusionFolder::intensityRaster(const lapis::Extent& e) const
	{
		return fineDataByExtentGeneric<int>(e, _layout, [&](size_t n) { return intensityRaster(n); });
	}

	std::optional<fs::path> FusionFolder::maxHeightRaster(size_t index) const
	{
		return _getTileMetric("segments_Max_Height_Map.img", index);
	}

	std::optional<lapis::Raster<double>> FusionFolder::maxHeightRaster(const lapis::Extent& e) const
	{
		return fineDataByExtentGeneric<double>(e, _layout, [&](size_t n) { return maxHeightRaster(n); });
	}

	std::optional<fs::path> FusionFolder::csmRaster(size_t index) const
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

	std::optional<lapis::Raster<double>> FusionFolder::csmRaster(const lapis::Extent& e) const
	{
		return fineDataByExtentGeneric<double>(e, _layout, [&](size_t n) { return csmRaster(n); });
	}

	lapis::CoordXY FusionFolder::coordGetter(const lapis::ConstFeature<lapis::MultiPolygon> ft) const {
		return { ft.getNumericField<lapis::coord_t>(_x), ft.getNumericField<lapis::coord_t>(_y) };
	}

	lapis::coord_t FusionFolder::heightGetter(const lapis::ConstFeature<lapis::MultiPolygon> ft) const {
		return ft.getNumericField<lapis::coord_t>(_h);
	}

	lapis::coord_t FusionFolder::radiusGetter(const lapis::ConstFeature<lapis::MultiPolygon> ft) const {
		return std::sqrt(areaGetter(ft) / M_PI);
	}

	lapis::coord_t FusionFolder::areaGetter(const lapis::ConstFeature<lapis::MultiPolygon> ft) const {
		return ft.getNumericField<lapis::coord_t>(_a);
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

	std::optional<fs::path> FusionFolder::_getTileMetric(const std::string& basename, size_t index) const
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