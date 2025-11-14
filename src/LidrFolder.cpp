#include "LidrFolder.hpp"

namespace processedfolder {


	LidRFolder::LidRFolder(const std::string& folder) {
		std::cout << "a\n";

		if (!fs::is_directory(folder)) {
			throw std::invalid_argument("Folder does not exist");
		}
		std::cout << "b\n";

		if (!fs::exists(fs::path(folder) / "Layout")) {
			throw std::invalid_argument("Not a lidR folder");
		}
		std::cout << "c\n";

		_folder = folder;
		_layout = sp::SpVectorDataset<sp::SpMultiPolygon>((_folder / "layout" / "layout.shp").string());
		std::cout << "1\n";

		if (!fs::exists(_folder / "mask")) {
			return;
		}
		std::cout << "2\n";

		_proj = spatial::Raster<int>(getMaskRaster().value()).projection();
		std::cout << "3\n";

		_layout.projection(_proj);
		std::cout << "success\n";
	}

	fs::path LidRFolder::getDir() const
	{
		return _folder;
	}

	std::optional<std::string> LidRFolder::getCover() const {
		return std::optional<std::string>();
	}
	std::optional<std::string> LidRFolder::getP95() const {
		return std::optional<std::string>();
	}
	std::optional<std::string> LidRFolder::getRumple() const {
		return std::optional<std::string>();
	}
	std::optional<std::string> LidRFolder::getP25() const {
		return std::optional<std::string>();
	}
	std::optional<std::string> LidRFolder::getMeanHeight() const {
		return std::optional<std::string>();
	}
	std::optional<std::string> LidRFolder::getStdDevHeight() const {
		return std::optional<std::string>();
	}
	std::optional<std::string> LidRFolder::getSlope(sp::coord_t radiusMeters) const {
		return std::optional<std::string>();
	}
	std::optional<std::string> LidRFolder::getAspect(sp::coord_t radiusMeters) const {
		return std::optional<std::string>();
	}
	std::optional<std::string> LidRFolder::getTPI(sp::coord_t radiusMeters) const {
		return std::optional<std::string>();
	}
	std::optional<std::string> LidRFolder::getMaskRaster() const {
		auto path = _folder / "mask" / "mask.tif";
		if (fs::exists(path))
			return path.string();
		return std::optional<std::string>();
	}
	std::optional<std::string> LidRFolder::getTileLayoutVector() const {
		auto path = _folder / "layout" / "layout.shp";
		if (fs::exists(path))
			return path.string();
		return std::optional<std::string>();
	}
	int LidRFolder::nTiles() const {
		return _layout.nFeatures();
	}
	const sp::CoordRef& LidRFolder::getProjection() const {
		return _proj;
	}
	sp::coord_t LidRFolder::getConvFactor() const {
		const std::optional<sp::LinearUnit>& u = _layout.projection().getXYLinearUnits();
		return sp::linearUnitPresets::meter.convertOneToThis(1, u);
	}
	std::optional<sp::Alignment> LidRFolder::getMetricAlignment() const {
		return std::optional<sp::Alignment>();
	}
	std::optional<sp::Extent> LidRFolder::extentByTile(int index) const {
		if (index < 0 || index >= nTiles()) {
			return std::optional<sp::Extent>();
		}
		return _layout.getFeature(index).geom;
	}
	lico::TaoList LidRFolder::readAllHighPoints(int nThreads, double fixedRadius) const {
		return lico::TaoList();
	}
	lico::TaoList LidRFolder::getHighPointsByTile(int i, double fixedRadius) const {
		if (i < 0 || i >= nTiles()) {
			return lico::TaoList();
		}
		std::string tileName = _layout.getFeature(i).getAttributeAsString("uniqueid");
		fs::path expected = _folder / "segments" / (tileName + "_taos.csv");
		if (fs::exists(expected)) {
			return expected.string();
		}
		else {
			try {
				tileName = std::to_string(_layout.getFeature(i).getAttributeAsInt("uniqueid"));
				expected = _folder / "segments" / (tileName + "_taos.csv");
				if (fs::exists(expected)) {
					return expected.string();
				}
			}
			catch (std::invalid_argument e) {
				std::cerr << "Did not find " << _layout.getFeature(i).getAttributeAsString("uniqueid") << " and did not find it as an int either.";
			}
		}

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
	std::optional<std::string> LidRFolder::getSegmentRaster(int index) const {
		if (index < 0 || index >= nTiles()) {
			return std::optional<std::string>();
		}
		std::string tileName = _layout.getFeature(index).getAttributeAsString("uniqueid");
		fs::path expected = _folder / "segments" / (tileName + "_segments.tif");
		if (fs::exists(expected)) {
			return expected.string();
		}
		else {
			try {
				tileName = std::to_string(_layout.getFeature(index).getAttributeAsInt("uniqueid"));
				expected = _folder / "segments" / (tileName + "_segments.tif");

				if (fs::exists(expected)) {
					return expected.string();
				}
			}
			catch (std::invalid_argument e) {
				std::cerr << "Did not find " << _layout.getFeature(index).getAttributeAsString("uniqueid") << " and did not find it as an int either.";
			}
		}
		return std::optional<std::string>();
	}
	std::optional<std::string> LidRFolder::getCanopyIntensityRaster(int index) const {
		return std::optional<std::string>();
	}
	std::optional<std::string> LidRFolder::getTopsRaster(int index) const {
		if (index < 0 || index >= nTiles()) {
			return std::optional<std::string>();
		}
		std::string tileName = _layout.getFeature(index).getAttributeAsString("uniqueid");
		fs::path expected = _folder / "segments" / (tileName + "_tops.tif");
		if (fs::exists(expected)) {
			return expected.string();
		}
		else {
			try {
				tileName = std::to_string(_layout.getFeature(index).getAttributeAsInt("uniqueid"));
				expected = _folder / "segments" / (tileName + "_tops.tif");
				if (fs::exists(expected)) {
					return expected.string();
				}
			}
			catch (std::invalid_argument e) {
				std::cerr << "Did not find " << _layout.getFeature(index).getAttributeAsString("uniqueid") << " and did not find it as an int either.";
			}
		}
		return std::optional<std::string>();
	}
	std::optional<std::string> LidRFolder::getMaxHeightRaster(int index) const {
		if (index < 0 || index >= nTiles()) {
			return std::optional<std::string>();
		}
		std::string tileName = _layout.getFeature(index).getAttributeAsString("uniqueid");
		fs::path expected = _folder / "segments" / (tileName + "_mhm.tif");
		if (fs::exists(expected)) {
			return expected.string();
		}
		else {
			try {
				tileName = std::to_string(_layout.getFeature(index).getAttributeAsInt("uniqueid"));
				expected = _folder / "segments" / (tileName + "_mhm.tif");
				if (fs::exists(expected)) {
					return expected.string();
				}
			}
			catch (std::invalid_argument e) {
				std::cerr << "Did not find " << _layout.getFeature(index).getAttributeAsString("uniqueid") << " and did not find it as an int either.";
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

	std::optional<std::string> LidRFolder::getCsmRaster(int index) const {
		if (index < 0 || index >= nTiles()) {
			return std::optional<std::string>();
		}
		std::string tileName = _layout.getFeature(index).getAttributeAsString("uniqueid");
		fs::path expected = _folder / "chm" / (tileName + "_chm.tif");
		if (fs::exists(expected)) {
			return expected.string();
		}
		else {
			try {
				tileName = std::to_string(_layout.getFeature(index).getAttributeAsInt("uniqueid"));
				expected = _folder / "chm" / (tileName + "_chm.tif");
				if (fs::exists(expected)) {
					return expected.string();
				}
			}
			catch (std::invalid_argument e) {
				std::cerr << "Did not find " << _layout.getFeature(index).getAttributeAsString("uniqueid") << " and did not find it as an int either.";
			}
		}
		return std::optional<std::string>();
	}
}