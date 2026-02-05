#include "LidRFolder.hpp"

namespace processedfolder {
	namespace fs = std::filesystem;

	LidRFolder::LidRFolder(const fs::path& folder) {
		if (!fs::is_directory(folder)) {
			throw std::invalid_argument("Folder does not exist");
		}
		if (!fs::exists(fs::path(folder) / "Layout")) {
			throw std::invalid_argument("Not a lidR folder");
		}
		_folder = folder;
		_layout = lapis::VectorDataset<lapis::MultiPolygon>((_folder / "layout" / "layout.shp").string());

		if (!fs::exists(_folder / "mask")) {
			return;
		}
		_proj = lapis::Raster<int>(maskRaster().value().string()).crs();
		_layout.projectInPlace(_proj);
	}

	const fs::path LidRFolder::dir() const
	{
		return _folder;
	}

	std::optional<fs::path> LidRFolder::cover(bool allReturns) const {
		return std::optional<fs::path>();
	}

	std::optional<fs::path> LidRFolder::p95(bool allReturns) const {
		return std::optional<fs::path>();
	}

	std::optional<fs::path> LidRFolder::rumple(bool allReturns) const {
		return std::optional<fs::path>();
	}

	std::optional<fs::path> LidRFolder::p25(bool allReturns) const {
		return std::optional<fs::path>();
	}

	std::optional<fs::path> LidRFolder::meanHeight(bool allReturns) const {
		return std::optional<fs::path>();
	}

	std::optional<fs::path> LidRFolder::stdDevHeight(bool allReturns) const {
		return std::optional<fs::path>();
	}

	std::optional<fs::path> LidRFolder::maskRaster(bool allReturns) const {
		if (!allReturns) {
			std::cout << "LidRFolder does not distinguish between first and all returns. AllReturns mask was used.\n";
		}
		auto path = _folder / "mask" / "mask.tif";
		if (fs::exists(path))
			return path.string();
		return std::optional<fs::path>();
	}

	std::optional<fs::path> LidRFolder::slope(lapis::coord_t radiusMeters, lapis::LinearUnit unit) const {
		return std::optional<fs::path>();
	}

	std::optional<fs::path> LidRFolder::aspect(lapis::coord_t radiusMeters, lapis::LinearUnit unit) const {
		return std::optional<std::string>();
	}

	std::optional<fs::path> LidRFolder::tpi(lapis::coord_t radiusMeters, lapis::LinearUnit unit) const {
		return std::optional<fs::path>();
	}

	const RunType LidRFolder::type() const {
		return RunType::lidr;
	}
	
	std::optional<fs::path> LidRFolder::tileLayoutVector() const {
		auto path = _folder / "layout" / "layout.shp";
		if (fs::exists(path))
			return path;
		return std::optional<fs::path>();
	}

	size_t LidRFolder::nTiles() const {
		return _layout.nFeature();
	}

	const lapis::CoordRef LidRFolder::crs() const {
		return _proj;
	}

	const lapis::Extent& LidRFolder::extent() const
	{
		return _layout.extent();
	}

	std::optional<lapis::LinearUnit> LidRFolder::units() const {
		return _layout.crs().getXYLinearUnits();
	}

	std::optional<lapis::Alignment> LidRFolder::metricAlignment() const {
		return std::optional<lapis::Alignment>();
	}

	std::optional<lapis::Alignment> LidRFolder::csmAlignment() const {
		return std::optional<lapis::Alignment>();
	}

	std::optional<lapis::Extent> LidRFolder::extentByTile(size_t index) const {
		if (index < 0 || index >= nTiles()) {
			return std::optional<lapis::Extent>();
		}
		return _layout.getFeature(index).getGeometry().boundingBox();
	}

	lapis::VectorDataset<lapis::Point> LidRFolder::allHighPoints() const {
		return lapis::VectorDataset<lapis::Point>();
	}

	std::optional<fs::path> LidRFolder::highPoints(size_t index) const {
		if (index < 0 || index >= nTiles()) {
			return std::optional<fs::path>();
		}

		std::string tileName = _layout.getStringField(index, "uniqueid");
		fs::path expected = _folder / "segments" / (tileName + "_highPoints.shp");
		if (fs::exists(expected)) {
			return expected;
		}
		else {
			try {
				tileName = std::to_string(std::stoi(tileName));
				expected = _folder / "segments" / (tileName + "_taos.shp");
				if (fs::exists(expected)) {
					return expected;
				}
			}
			catch (std::invalid_argument e) {
				std::cerr << "Did not find " << _layout.getStringField(index, "uniqueid") << " and did not find it as an int either, creating points.\n";
			}
		}

		try {
			lapis::VectorDataset<lapis::Point> out{};
			out.addNumericField<lapis::coord_t>("X");
			out.addNumericField<lapis::coord_t>("Y");
			out.addNumericField<lapis::csm_t>("Height");
			out.addNumericField<lapis::coord_t>("Area");

			auto basin = lapis::Raster<int>(stringOrThrow(watershedSegmentRaster(index)));
			auto chm = lapis::Raster<lapis::csm_t>(stringOrThrow(csmRaster(index)));
			auto tops = lapis::Raster<uint8_t>(stringOrThrow(topsRaster(index)));

			std::map<int, lapis::csm_t> map;
			for (lapis::cell_t c = 0; c < basin.ncell(); c++) {
				if (basin[c].has_value()) {
					map.emplace(basin[c].value(), 0);
					++map[basin[c].value()];
				}
			}
			auto convarea = basin.xres() * basin.yres();
			for (lapis::cell_t c = 0; c < tops.ncell(); c++) {
				if (tops[c].has_value() && tops[c].value()) {
					lapis::Point pt{ tops.xFromCell(c), tops.yFromCell(c) };
					out.addGeometry(pt);
					out.back().setNumericField<lapis::coord_t>("X", pt.x());
					out.back().setNumericField<lapis::coord_t>("Y", pt.y());
					out.back().setNumericField<lapis::csm_t>("Height", chm[c].value());
					out.back().setNumericField<lapis::coord_t>("Area", map[basin[c].value()] * convarea);
				}
			}
			out.writeShapefile(expected.string());
		}
		catch (FileNotFoundException e) {
			std::cerr << "Did not find data for" << _layout.getStringField(index, "uniqueid") << ".\n";
			return std::optional<fs::path>();

		}
		return expected;
	}

	lapis::VectorDataset<lapis::Point> LidRFolder::highPoints(const lapis::Extent& e) const {
		lapis::VectorDataset<lapis::Point> full{};

		lapis::Extent projE = lapis::QuadExtent(e, _layout.crs()).outerExtent();
		if (!projE.overlaps(_layout.extent())) {
			return full;
		}

		for (size_t i = 0; i < _layout.nFeature(); ++i) {
			std::optional<fs::path> filePath = highPoints(i);
			if (filePath) {
				if (!full.nFeature()) {
					full = lapis::VectorDataset<lapis::Point>(filePath.value());
				}
				else {
					full.appendFile(filePath.value());
				}
			}
		}
		auto out = lapis::emptyVectorDatasetFromTemplate(full);
		for (lapis::ConstFeature<lapis::Point> ft : full) {
			if (e.contains(ft.getGeometry().x(), ft.getGeometry().y())) {
				out.addFeature(ft);
			}
		}
		return out;
	}

	lapis::VectorDataset<lapis::MultiPolygon> LidRFolder::allPolygons() const {
		lapis::VectorDataset<lapis::MultiPolygon> out{};
		auto ntile = nTiles();
		std::optional<fs::path> file;
		std::vector<std::filesystem::path> files;
		for (size_t cell = 0; cell < ntile; ++cell) {
			file = polygons(cell);
			if (file) {
				files.push_back(*file);
			}
		}
		out = lapis::VectorDataset<lapis::MultiPolygon>(files);
		return out;
	}

	lapis::VectorDataset<lapis::MultiPolygon> LidRFolder::polygons(const lapis::Extent& e) const {
		lapis::VectorDataset<lapis::MultiPolygon> out{};
		bool outInit = false;

		lapis::Extent projE = lapis::QuadExtent(e, _layout.crs()).outerExtent();
		if (!projE.overlaps(_layout.extent())) {
			return out;
		}

		for (size_t i = 0; i < _layout.nFeature(); ++i) {
			std::optional<fs::path> filePath = polygons(i);
			if (filePath) {
				auto tileExtent = extentByTile(i).value();
				if (!projE.overlaps(tileExtent)) {
					continue;
				}
				lapis::VectorDataset<lapis::MultiPolygon> thisPolygons{ filePath.value() };
				if (thisPolygons.nFeature()) {
					if (!outInit) {
						out = lapis::emptyVectorDatasetFromTemplate(thisPolygons);
					}

					for (lapis::ConstFeature<lapis::MultiPolygon> ft : thisPolygons) {
						if (projE.contains(ft.getNumericField<lapis::coord_t>("X"), ft.getNumericField<lapis::coord_t>("Y"))) {
							if (tileExtent.contains(ft.getNumericField<lapis::coord_t>("X"), ft.getNumericField<lapis::coord_t>("Y"))) {
								out.addFeature(ft);
							}
						}
					}
				}
			}
		}
		return out;
	}

	std::optional<fs::path> LidRFolder::polygons(size_t index) const {
		if (index < 0 || index >= nTiles()) {
			return std::optional<fs::path>();
		}

		std::string tileName = _layout.getStringField(index, "uniqueid");
		fs::path expected = _folder / "segments" / (tileName + "_polygons.shp");
		if (fs::exists(expected)) {
			return expected;
		}
		else {
			try {
				tileName = std::to_string(std::stoi(tileName));
				expected = _folder / "segments" / (tileName + "_taos.shp");
				if (fs::exists(expected)) {
					return expected;
				}
			}
			catch (std::invalid_argument e) {
				std::cerr << "Did not find " << _layout.getStringField(index, "uniqueid") << " and did not find it as an int either, creating points.\n";
			}
		}

		try {
			auto basin = lapis::Raster<int>(stringOrThrow(watershedSegmentRaster(index)));
			auto chm = lapis::Raster <lapis::coord_t> (stringOrThrow(csmRaster(index)));
			auto tops = lapis::Raster<uint8_t>(stringOrThrow(topsRaster(index)));

			auto out = lapis::rasterToMultiPolygonForTaos(basin);
			out.addNumericField<lapis::coord_t>("X");
			out.addNumericField<lapis::coord_t>("Y");
			out.addNumericField<lapis::csm_t>("Height");
			out.addNumericField<lapis::coord_t>("Area");

			std::unordered_map<int, lapis::csm_t> aMap;
			for (lapis::cell_t c = 0; c < basin.ncell(); c++) {
				if (basin[c].has_value()) {
					aMap.emplace(basin[c].value(), 0);
					++aMap[basin[c].value()];
				}
			}
			
			std::unordered_map<int, lapis::coord_t> xMap;
			std::unordered_map<int, lapis::coord_t> yMap;
			std::unordered_map<int, lapis::coord_t> hMap;
			for (lapis::cell_t c = 0; c < tops.ncell(); ++c) {
				if (tops[c].has_value() && tops[c].value()) {
					auto x = tops.xFromCell(c);
					auto y = tops.yFromCell(c);
					basin.extract(x, y, lapis::ExtractMethod::near);
					/*if (id.has_value()) {
						auto h = chm.extract(x, y, lapis::ExtractMethod::near);
						xMap.emplace(id.value(), x);
						yMap.emplace(id.value(), y);
						hMap.emplace(id.value(), h.value());
					}*/
				}
			}
			
			auto convarea = basin.xres() * basin.yres();
			for (auto ft : out) {
				auto id = ft.getNumericField<int>("ID");
				ft.setNumericField<lapis::coord_t>("X", xMap[id]);
				ft.setNumericField<lapis::coord_t>("y", yMap[id]);
				ft.setNumericField<lapis::csm_t>("Height", hMap[id]);
				ft.setNumericField<lapis::coord_t>("Area", aMap[id]);
			}
			out.writeShapefile(expected.string());
		}
		catch (FileNotFoundException e) {
			std::cerr << "Did not find data for" << _layout.getStringField(index, "uniqueid") << ".\n";
			return std::optional<fs::path>();

		}
		return expected;
	}

	template<class T>
	std::optional<lapis::Raster<T>> fineDataByExtentGeneric(const lapis::Extent& e, const lapis::VectorDataset<lapis::MultiPolygon>& tileLayout, std::function<std::optional<fs::path>(size_t)> byTile) {
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
				out->overlay(tile, [](T a, T b) {return a; });
			}
			catch (lapis::LapisGisException e) {
				continue;
			}
		}
		return out;
	}

	std::optional<fs::path> LidRFolder::topsRaster(size_t index) const {
		if (index < 0 || index >= nTiles()) {
			return std::optional<std::string>();
		}
		std::string tileName = _layout.getStringField(index, "uniqueid");
		fs::path expected = _folder / "segments" / (tileName + "_tops.tif");
		if (fs::exists(expected)) {
			return expected.string();
		}
		else {
			try {
				tileName = std::to_string(std::stoi(tileName));
				expected = _folder / "segments" / (tileName + "_tops.tif");
				if (fs::exists(expected)) {
					return expected.string();
				}
			}
			catch (std::invalid_argument e) {
				std::cerr << "Did not find " << tileName << " and did not find it as an int either.";
			}
		}
		return std::optional<std::string>();
	}

	std::optional<lapis::Raster<uint8_t>> LidRFolder::topsRaster(const lapis::Extent& e) const {
		return fineDataByExtentGeneric<uint8_t>(e, _layout, [&](size_t n) { return topsRaster(n); });
	}

	std::optional<fs::path> LidRFolder::watershedSegmentRaster(size_t index) const {
		if (index < 0 || index >= nTiles()) {
			return std::optional<fs::path>();
		}
		std::string tileName = _layout.getStringField(index, "uniqueid");
		fs::path expected = _folder / "segments" / (tileName + "_segments.tif");
		if (fs::exists(expected)) {
			return expected.string();
		}
		else {
			try {
				tileName = std::to_string(std::stoi(tileName));
				expected = _folder / "segments" / (tileName + "_segments.tif");

				if (fs::exists(expected)) {
					return expected.string();
				}
			}
			catch (std::invalid_argument e) {
				std::cerr << "Did not find " << tileName << " and did not find it as an int either.";
			}
		}
		return std::optional<fs::path>();
	}

	std::optional<lapis::Raster<lapis::taoid_t>> LidRFolder::watershedSegmentRaster(const lapis::Extent& e) const {
		return fineDataByExtentGeneric<lapis::taoid_t>(e, _layout, [&](size_t n) { return watershedSegmentRaster(n); });
	}

	std::optional<fs::path> LidRFolder::intensityRaster(size_t index) const {
		return std::optional<fs::path>();
	}

	std::optional<lapis::Raster<lapis::intensity_t>> LidRFolder::intensityRaster(const lapis::Extent& e) const {
		return std::optional<lapis::Raster<lapis::intensity_t>>();
	}

	std::optional<fs::path> LidRFolder::maxHeightRaster(size_t index) const {
		if (index < 0 || index >= nTiles()) {
			return std::optional<fs::path>();
		}
		std::string tileName = _layout.getStringField(index, "uniqueid");
		fs::path expected = _folder / "segments" / (tileName + "_mhm.tif");
		if (fs::exists(expected)) {
			return expected.string();
		}
		else {
			try {
				tileName = std::to_string(std::stoi(tileName));
				expected = _folder / "segments" / (tileName + "_mhm.tif");
				if (fs::exists(expected)) {
					return expected.string();
				}
			}
			catch (std::invalid_argument e) {
				std::cerr << "Did not find " << tileName << " and did not find it as an int either.";
			}
		}

		std::cout << "creating mhm for first time access\n";
		auto basin = lapis::Raster<int>(stringOrThrow(watershedSegmentRaster(index)));
		auto chm = lapis::Raster<lapis::csm_t>(stringOrThrow(csmRaster(index)));
		auto tops = lapis::Raster<int>(stringOrThrow(topsRaster(index)));

		std::map<int, lapis::csm_t> map;
		for (lapis::cell_t c = 0; c < tops.ncell(); c++) {
			if (tops[c].has_value() && tops[c].value() == 1) {
				auto b = basin[c].value();
				auto ht = chm[c].value();
				map.emplace(b, ht);
			}
		}
		auto out = chm;
		for (lapis::cell_t c = 0; c < out.ncell(); c++) {
			if (basin[c].has_value()) {
				out[c].value() = map[basin[c].value()];
			}
		}
		out.writeRaster(expected.string());
		return expected.string();
	}

	std::optional<lapis::Raster<lapis::csm_t>> LidRFolder::maxHeightRaster(const lapis::Extent& e) const {
		return fineDataByExtentGeneric<lapis::csm_t>(e, _layout, [&](size_t n) { return maxHeightRaster(n); });
	}

	std::optional<fs::path> LidRFolder::csmRaster(size_t index) const {
		if (index < 0 || index >= nTiles()) {
			return std::optional<std::string>();
		}
		std::string tileName = _layout.getStringField(index, "uniqueid");
		fs::path expected = _folder / "chm" / (tileName + "_chm.tif");
		if (fs::exists(expected)) {
			return expected.string();
		}
		else {
			try {
				tileName = std::to_string(std::stoi(tileName));
				expected = _folder / "chm" / (tileName + "_chm.tif");
				if (fs::exists(expected)) {
					return expected.string();
				}
			}
			catch (std::invalid_argument e) {
				std::cerr << "Did not find " << tileName << " and did not find it as an int either.";
			}
		}
		return std::optional<std::string>();
	}

	std::optional<lapis::Raster<lapis::csm_t>> LidRFolder::csmRaster(const lapis::Extent& e) const {
		return fineDataByExtentGeneric<lapis::csm_t>(e, _layout, [&](size_t n) { return csmRaster(n); });
	}

	std::function<lapis::CoordXY(const lapis::ConstFeature<lapis::MultiPolygon>&)> LidRFolder::coordGetter() const {
		return [](const lapis::ConstFeature<lapis::MultiPolygon>& ft)->lapis::CoordXY {
			return lapis::CoordXY(ft.getNumericField<lapis::coord_t>("X"), ft.getNumericField<lapis::coord_t>("Y"));
			};
	}

	std::function<lapis::coord_t(const lapis::ConstFeature<lapis::MultiPolygon>&)> LidRFolder::heightGetter() const {
		return [](const lapis::ConstFeature<lapis::MultiPolygon>& ft)->lapis::coord_t {
			return ft.getNumericField<lapis::coord_t>("Height");
			};
	}

	std::function<lapis::coord_t(const lapis::ConstFeature<lapis::MultiPolygon>&)> LidRFolder::radiusGetter() const {
		return [](const lapis::ConstFeature<lapis::MultiPolygon>& ft)->lapis::coord_t {
			return std::sqrt(ft.getNumericField<lapis::coord_t>("Area"));
			};
	}

	std::function<lapis::coord_t(const lapis::ConstFeature<lapis::MultiPolygon>&)> LidRFolder::areaGetter() const {
		return [](const lapis::ConstFeature<lapis::MultiPolygon>& ft)->lapis::coord_t {
			return ft.getNumericField<lapis::coord_t>("Area");
			}; 
	}
}