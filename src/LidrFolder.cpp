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
		_layout.projectInPlacePreciseExtent(_proj);
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
	
	std::optional<fs::path> LidRFolder::tileLayoutVector() const {
		auto path = _folder / "layout" / "layout.shp";
		if (fs::exists(path))
			return path;
		return std::optional<fs::path>();
	}

	int LidRFolder::nTiles() const {
		return _layout.nFeature();
	}

	const lapis::CoordRef LidRFolder::crs() const {
		return _proj;
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

	std::optional<lapis::Extent> LidRFolder::extentByTile(int index) const {
		if (index < 0 || index >= nTiles()) {
			return std::optional<lapis::Extent>();
		}
		return _layout.getFeature(index).getGeometry().boundingBox();
	}

	lapis::VectorDataset<lapis::Point> LidRFolder::allHighPoints() const {
		return lapis::VectorDataset<lapis::Point>();
	}

	std::optional<fs::path> LidRFolder::highPoints(int index) const {
		this code should create a shapefile and return the path to that;
		if (index < 0 || index >= nTiles()) {
			return std::optional<fs::path>();
		}

		std::string tileName = _layout.getStringField(index, "uniqueid");
		fs::path expected = _folder / "segments" / (tileName + "_taos.csv");
		if (fs::exists(expected)) {
			return expected;
		}
		else {
			try {
				tileName = std::to_string(std::stoi(tileName));
				expected = _folder / "segments" / (tileName + "_taos.csv");
				if (fs::exists(expected)) {
					return expected;
				}
			}
			catch (std::invalid_argument e) {
				std::cerr << "Did not find " << _layout.getStringField(index, "uniqueid") << " and did not find it as an int either.";
			}
		}
		return std::optional<fs::path>();

		auto tl = lico::TaoList();

		auto basin = sp::Raster<int>(stringOrThrow(getSegmentRaster(i)));
		auto chm = sp::Raster<double>(stringOrThrow(getCsmRaster(i)));
		auto tops = sp::Raster<int>(stringOrThrow(getTopsRaster(i)));

		std::map<int, double> map;
		for (sp::cell_t c = 0; c < basin.ncell(); c++) {
			if (basin[c].has_value()) {
				map.emplace(basin[c].value(), 0);
				++map[basin[c].value()];
			}
		}
		auto convarea = basin.xres() * basin.yres();
		for (sp::cell_t c = 0; c < tops.ncell(); c++) {
			if (tops[c].has_value() && tops[c].value() == 1) {
				auto r = 0;
				if (fixedRadius > 0)
					r = fixedRadius;
				else
					r = std::sqrt((map[basin[c].value()] * convarea) / M_PI);

				auto tao = lico::Tao(tops.xFromCell(c), tops.yFromCell(c), chm[c].value(), r, map[basin[c].value()] * convarea);
				tl.addTAO(tao);
			}
		}
		tl.writeCsv(expected.string());
		return tl;
	}

	lapis::VectorDataset<lapis::Point> LidRFolder::highPoints(const lapis::Extent& e) const {
		sda;
	}

	std::optional<fs::path> LidRFolder::topsRaster(int index) const {
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

	std::optional<lapis::Raster<int>> LidRFolder::topsRaster(const  lapis::Extent& e) const {
		asd;
	}

	std::optional<fs::path> LidRFolder::watershedSegmentRaster(int index) const {
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

	std::optional<fs::path> LidRFolder::intensityRaster(int index) const {
		return std::optional<fs::path>();
	}

	std::optional<lapis::Raster<int>> LidRFolder::intensityRaster(const lapis::Extent& e) const {
		return std::optional<lapis::Raster<int>>();
	}

	std::optional<fs::path> LidRFolder::maxHeightRaster(int index) const {
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
		auto basin = sp::Raster<int>(stringOrThrow(getSegmentRaster(index)));
		auto chm = sp::Raster<double>(stringOrThrow(getCsmRaster(index)));
		auto tops = sp::Raster<int>(stringOrThrow(getTopsRaster(index)));

		std::map<int, double> map;
		for (sp::cell_t c = 0; c < tops.ncell(); c++) {
			if (tops[c].has_value() && tops[c].value() == 1) {
				auto b = basin[c].value();
				auto ht = chm[c].value();
				map.emplace(b, ht);
			}
		}
		auto out = basin;
		for (sp::cell_t c = 0; c < out.ncell(); c++) {
			if (basin[c].has_value()) {
				out[c].value() = map[basin[c].value()];
			}
		}
		out.writeRaster(expected.string());
		return expected.string();
	}

	std::optional<lapis::Raster<int>> LidRFolder::maxHeightRaster(const lapis::Extent& e) const {
		asd;
	}

	std::optional<fs::path> LidRFolder::csmRaster(int index) const {
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

	std::optional<lapis::Raster<double>> csmRaster(const lapis::Extent& e) const {
		asd;
	}
}