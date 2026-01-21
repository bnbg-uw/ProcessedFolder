#include "LapisFolder.hpp"

namespace processedfolder {
	namespace fs = std::filesystem;

	static fs::path constructMetricFilePath(const fs::path& folder, const std::string& runName,
		const std::string& baseName, const std::string& unitName) {
		std::string fileName = baseName;
		if (runName.size()) {
			fileName = runName + "_" + baseName;
		}
		if (unitName.length()) {
			fileName += "_" + unitName;
		}
		fileName += ".tif";
		return folder / fileName;
	}

	static fs::path constructFullPathTif(const fs::path& folder, const std::string& runName,
		const std::string& tileName, const std::string& baseName, const std::string& unitName) {
		std::string fileName = baseName;
		if (runName.size()) {
			fileName = runName + "_" + baseName;
		}
		fileName = fileName + "_" + tileName;
		if (unitName.length()) {
			fileName += "_" + unitName;
		}
		fileName += ".tif";
		return folder / fileName;
	}

	static std::optional<fs::path> checkAllUnits(const fs::path& folder, const std::string& runName,
		const std::string& tileName, const std::string& baseName) {
		fs::path candidate = constructFullPathTif(folder, runName, tileName, baseName, "");
		if (exists(candidate)) {
			return candidate;
		}
		candidate = constructFullPathTif(folder, runName, tileName, baseName, "Meters");
		if (exists(candidate)) {
			return candidate;
		}
		candidate = constructFullPathTif(folder, runName, tileName, baseName, "Feet");
		if (exists(candidate)) {
			return candidate;
		}
		return std::optional<fs::path>();
	}

	LapisFolder::LapisFolder(const fs::path& folder)
	{
		namespace po = boost::program_options;
		using namespace lapis;

		if (!isLapisFolder(folder)) {
			throw std::runtime_error(folder.string() + " is not a Lapis run folder.");
		}
		_folder = folder;



		fs::path iniFile = folder / "RunParameters" / "FullParameters.ini";
		try {
			po::options_description nameGetter;
			nameGetter.add_options()("name", po::value<std::string>(&_name));
			po::variables_map vmFull;
			po::store(po::parse_config_file(iniFile.string().c_str(), nameGetter, true), vmFull);
			po::notify(vmFull);
			_name = vmFull.at("name").as<std::string>();
		}
		catch (...) {
			throw std::runtime_error(folder.string() + " has an issue in FullParameters.ini");
		}

		auto extentFromLayer = [](OGRLayer* layer) {
			OGREnvelope envelope;
			layer->GetExtent(&envelope, true);
			OGRSpatialReference* sr = layer->GetSpatialRef();
			CoordRef crs = CoordRef(sr);
			Extent e{ envelope.MinX,envelope.MaxX,envelope.MinY,envelope.MaxY,crs };
			return e;
			};
		auto extentFromFeature = [](OGRFeature& feature) {
			OGRGeometry* geom = feature.GetGeometryRef();
			OGREnvelope envelope;
			geom->getEnvelope(&envelope);
			//a more generalized variant of this would give this a CoordRef too, but I don't need it here, and it's a bit expensive
			Extent e{ envelope.MinX,envelope.MaxX,envelope.MinY,envelope.MaxY };
			return e;
			};

		fs::path layoutFile = folder / "Layout" / "TileLayout.shp";
		UniqueGdalDataset layout = vectorGDALWrapper(layoutFile.string());
		OGRLayer* layer = layout->GetLayer(0);
		Extent fullExtent = extentFromLayer(layer);

		auto closeToInteger = [](coord_t x) {
			x = fmod(x, 1.);
			x = std::min(x, 1 - x);
			return x < 0.01;
			};

		bool init = false;
		for (const auto& feature : layer) {
			Extent e = extentFromFeature(*feature);
			if (!init) {
				coord_t nCol = fullExtent.xspan() / e.xspan();
				coord_t nRow = fullExtent.yspan() / e.yspan();
				if (!closeToInteger(nCol) || !closeToInteger(nRow)) {
					throw std::runtime_error("Issue with tile size in layout file.");
				}
				_layoutRaster = Raster<bool>{ Alignment(fullExtent.xmin(), fullExtent.ymin(), (rowcol_t)std::round(nRow), (rowcol_t)std::round(nCol), e.xspan(), e.yspan(), fullExtent.crs()) };
				init = true;
			}
			coord_t xcenter = e.xmin() + e.xspan() / 2.;
			coord_t ycenter = e.ymin() + e.yspan() / 2.;
			auto v = _layoutRaster.atXYUnsafe(xcenter, ycenter);
			v.has_value() = true;
			v.value() = true;
		}
	}

	const fs::path LapisFolder::dir() const
	{
		return _folder;
	}

	const std::string& LapisFolder::name() const
	{
		return _name;
	}

	std::optional<fs::path> LapisFolder::cover(bool allReturns) const
	{
		return _getMetricByName("CanopyCover", allReturns);
	}

	std::optional<fs::path> LapisFolder::p95(bool allReturns) const
	{
		return _getMetricByName("95thPercentile_CanopyHeight", allReturns); 
	}

	std::optional<fs::path> LapisFolder::rumple(bool allReturns) const
	{
		return _getMetricByName("RumpleCSM", allReturns);
	}

	std::optional<fs::path> LapisFolder::p25(bool allReturns) const
	{
		return _getMetricByName("25thPercentile_CanopyHeight", allReturns);
	}

	std::optional<fs::path> LapisFolder::meanHeight(bool allReturns) const
	{
		return _getMetricByName("Mean_CanopyHeight", allReturns);
	}

	std::optional<fs::path> LapisFolder::stdDevHeight(bool allReturns) const
	{
		return _getMetricByName("StdDev_CanopyHeight", allReturns);
	}

	std::optional<fs::path> LapisFolder::slope(lapis::coord_t radius, lapis::LinearUnit unit) const
	{
		if (radius > 0) {
			std::cout << "Warning: Radius discarded as Lapis does not use a radius for slope\n";
		}
		return _getMetricByName("Slope");
	}

	std::optional<fs::path> LapisFolder::aspect(lapis::coord_t radius, lapis::LinearUnit unit) const
	{
		if (radius > 0) {
			std::cout << "Warning: Radius discarded as Lapis does not use a radius for aspect\n";
		}
		return _getMetricByName("Aspect");
	}

	std::optional<fs::path> LapisFolder::tpi(lapis::coord_t radius, lapis::LinearUnit unit) const
	{
		auto converter = lapis::LinearUnitConverter(unit, units());
		radius = converter(radius);

		fs::path topoFolder = _folder / "Topography";
		if (!fs::exists(topoFolder)) {
			return std::optional<fs::path>();
		}
		std::regex deleteBefore{ "^" + _name + "_TopoPositionIndex_" };
		std::regex deleteAfter{ "(Meters_Meters|Feet_Feet)\\.tif$" };
		for (const auto& fn : fs::directory_iterator(topoFolder)) {
			std::string scaleName = fn.path().filename().string();
			scaleName = std::regex_replace(scaleName, deleteBefore, "");
			scaleName = std::regex_replace(scaleName, deleteAfter, "");
			try {
				lapis::coord_t scale = std::stod(scaleName);
				if (std::abs(scale - radius) < 1) {
					return fn.path();
				}
			}
			catch (std::invalid_argument e) {
				continue;
			}
		}
		return std::optional<fs::path>();
	}

	std::optional<fs::path> LapisFolder::maskRaster(bool allReturns) const
	{
		return _getMetricByName("TotalReturnCount", allReturns);
	}

	std::optional<fs::path> LapisFolder::heightPercentile(int percentile, bool allReturns) const {
		std::string name = std::to_string(percentile);
		if (name.size() == 1) {
			name = "0" + name;
		}
		name = name + "Percentile_CanopyHeight";
		return _getMetricByName(name, allReturns);
	}

	const RunType LapisFolder::type() const {
		return RunType::lapis;
	}

	std::optional<fs::path> LapisFolder::tileLayoutVector() const
	{
		fs::path layoutPath = _folder / "Layout" / "TileLayout.shp";
		if (fs::exists(layoutPath)) {
			return layoutPath;
		}
		return std::optional<fs::path>();
	}

	size_t LapisFolder::nTiles() const
	{
		return _layoutRaster.ncell();
	}


	size_t LapisFolder::nTilesWithData() const
	{
		size_t out = 0;
		for (auto cell : lapis::CellIterator(_layoutRaster)) {
			if (_layoutRaster[cell].value()) {
				out++;
			}
		}
		return out;
	}

	std::string LapisFolder::tileNameFromTile(size_t n) const
	{
		if (n < 0 || n >= (size_t)_layoutRaster.ncell()) {
			return "Error";
		}
		return tileNameFromTile(_layoutRaster.rowFromCellUnsafe(n), _layoutRaster.colFromCellUnsafe(n));
	}

	std::string LapisFolder::tileNameFromTile(lapis::rowcol_t row, lapis::rowcol_t col) const
	{
		if (col < 0 || row < 0 || col >= _layoutRaster.ncol() || row >= _layoutRaster.nrow()) {
			return "Error";
		}
		auto padZeroes = [](int n, int nDigits) {
			std::string out = std::to_string(n);
			while (out.size() < nDigits) {
				out = "0" + out;
			}
			return out;
			};
		int nDigits = (int)std::ceil(std::log10(std::max(_layoutRaster.ncol(), _layoutRaster.nrow())));
		return "Col" + padZeroes(col + 1, nDigits) + "_Row" + padZeroes(row + 1, nDigits); //the names are 1-indexed, but it's 0-indexed internally
	}

	bool LapisFolder::shouldTileExist(size_t n) const
	{
		if (n < 0 || n >= (size_t)_layoutRaster.ncell()) {
			return false;
		}
		return _layoutRaster.atCellUnsafe(n).has_value();
	}

	bool LapisFolder::shouldTileExist(lapis::rowcol_t row, lapis::rowcol_t col) const
	{
		if (row < 0 || col < 0 || row >= _layoutRaster.nrow() || col >= _layoutRaster.ncol()) {
			return false;
		}
		return _layoutRaster.atRCUnsafe(row, col).has_value();
	}

	const lapis::CoordRef LapisFolder::crs() const
	{
		return _layoutRaster.crs();
	}

	std::optional<lapis::LinearUnit> LapisFolder::units() const
	{
		return crs().getXYLinearUnits();
	}

	std::optional<lapis::Alignment> LapisFolder::metricAlignment() const {
		auto checkCandidate = [&](const fs::path& c) {
			if (!fs::exists(c)) {
				return std::optional<lapis::Alignment>();
			}
			std::optional<lapis::Alignment> a = lapis::Alignment(c.string());
			a->defineCRS(crs());
			return a;
			};
		fs::path candidateOne = constructMetricFilePath(dir() / "PointMetrics", name(), "TotalReturnCount", "");
		fs::path candidateTwo = constructMetricFilePath(dir() / "PointMetrics" / "AllReturns", name(), "TotalReturnCount", "");
		fs::path candidateThree = constructMetricFilePath(dir() / "PointMetrics" / "FirstReturn", name(), "TotalReturnCount", "");
		std::optional<lapis::Alignment> checked = checkCandidate(candidateOne);
		if (checked) {
			return checked;
		}
		checked = checkCandidate(candidateTwo);
		if (checked) {
			return checked;
		}
		checked = checkCandidate(candidateThree);
		return checked; //even if it's empty, that's the correct thing to return if all three fail
	}

	std::optional<lapis::Alignment> LapisFolder::csmAlignment() const
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

	lapis::Alignment LapisFolder::layoutAlignment() const {
		return lapis::Alignment{ _layoutRaster };
	}

	const lapis::Extent& LapisFolder::extent() const
	{
		return _layoutRaster;
	}


	std::optional<lapis::Extent> LapisFolder::extentByTile(size_t index) const
	{
		if (index < 0) {
			return std::optional<lapis::Extent>();
		}
		if (index >= (size_t)_layoutRaster.ncell()) {
			return std::optional<lapis::Extent>();
		}
		if (!_layoutRaster[index].value()) {
			return std::optional<lapis::Extent>();
		}
		return _layoutRaster.extentFromCell(index);
	}

	std::optional<lapis::Extent> LapisFolder::extentByTile(lapis::rowcol_t row, lapis::rowcol_t col) const
	{
		if (row < 0 || col < 0 || row >= _layoutRaster.nrow() || col >= _layoutRaster.ncol()) {
			return std::optional<lapis::Extent>();
		}
		return extentByTile(_layoutRaster.cellFromRowColUnsafe(row, col));
	}

	lapis::VectorDataset<lapis::Point> LapisFolder::allHighPoints() const
	{
		lapis::VectorDataset<lapis::Point> out{};
		auto ntile = nTiles();
		std::optional<fs::path> file;
		for (size_t cell = 0; cell < ntile; ++cell) {
			file = highPoints(cell);
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

	std::optional<fs::path> LapisFolder::highPoints(size_t index) const
	{
		if (index < 0 || index >= (size_t)_layoutRaster.ncell()) {
			return std::optional<fs::path>();
		}

		std::string fileName = "TAOs";
		if (name().size()) {
			fileName = name() + "_" + fileName;
		}
		fileName = fileName + "_" + tileNameFromTile(index) + ".shp";

		fs::path candidate = _folder / "TreeApproximateObjects" / fileName;
		if (fs::exists(candidate)) {
			return candidate;
		}

		candidate = _folder / "TreeApproximateObjects" / "Points" / fileName;
		if (fs::exists(candidate)) {
			return candidate;
		}

		return std::optional<fs::path>();
	}

	std::optional<fs::path> LapisFolder::highPoints(lapis::rowcol_t row, lapis::rowcol_t col) const
	{
		if (row < 0 || row >= _layoutRaster.nrow() || col < 0 || col >= _layoutRaster.ncol()) {
			return std::optional<fs::path>();
		}

		return highPoints(_layoutRaster.cellFromRowColUnsafe(row, col));
	}

	lapis::VectorDataset<lapis::Point> LapisFolder::highPoints(const lapis::Extent& e) const
	{
		lapis::VectorDataset<lapis::Point> full{};

		lapis::Extent projE = lapis::QuadExtent(e, _layoutRaster.crs()).outerExtent();
		if (!projE.overlaps(_layoutRaster)) {
			return full;
		}

		for (auto cell : lapis::CellIterator(_layoutRaster, projE, lapis::SnapType::out)) {
			std::optional<fs::path> filePath = highPoints(cell);
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
		for (auto ft : full) {
			if (e.contains(ft.getGeometry().x(), ft.getGeometry().y())) {
				out.addFeature(ft);
			}
		}
		return out;
	}

	std::optional<fs::path> LapisFolder::mcGaugheyPolygons(size_t index) const
	{
		if (index < 0 || index >= (size_t)_layoutRaster.ncell()) {
			return std::optional<fs::path>();
		}

		auto getName = [&](const std::string& baseName) {
			return name() + "_" + baseName + "_" + tileNameFromTile(index) + ".shp";
			};

		fs::path candidate = _folder / "TreeApproximateObjects" / "FusionPolygons" / getName("FusionPolygons");
		if (fs::exists(candidate)) {
			return candidate;
		}
		candidate = _folder / "TreeApproximateObjects" / "McGaugheyPolygons" / getName("McGaugheyPolygons");
		if (fs::exists(candidate)) {
			return candidate;
		}
		candidate = _folder / "TreeApproximateObjects" / "McGaugheyPolygons" / "SegmentPolygons" / getName("McGaugheyPolygons");
		if (fs::exists(candidate)) {
			return candidate;
		}
		return std::optional<fs::path>();
	}

	std::optional<fs::path> LapisFolder::mcGaugheyPolygons(lapis::rowcol_t row, lapis::rowcol_t col) const
	{
		if (row < 0 || row >= _layoutRaster.nrow() || col < 0 || col >= _layoutRaster.ncol()) {
			return std::optional<fs::path>();
		}

		return mcGaugheyPolygons(_layoutRaster.cellFromRowColUnsafe(row, col));
	}

	lapis::VectorDataset<lapis::MultiPolygon> LapisFolder::mcGaugheyPolygons(const lapis::Extent& e) const
	{
		lapis::VectorDataset<lapis::MultiPolygon> out{};
		bool outInit = false;

		lapis::Extent projE = lapis::QuadExtent(e, _layoutRaster.crs()).outerExtent();
		if (!projE.overlaps(_layoutRaster)) {
			return out;
		}

		for (auto cell : lapis::CellIterator(_layoutRaster, projE, lapis::SnapType::out)) {
			std::optional<fs::path> filePath = mcGaugheyPolygons(cell);
			if (filePath) {
				lapis::VectorDataset<lapis::MultiPolygon> thisPolygons{ filePath.value() };
				if (thisPolygons.nFeature()) {
					if (!outInit) {
						out = lapis::emptyVectorDatasetFromTemplate(thisPolygons);
						outInit = true;
					}

					for (lapis::ConstFeature<lapis::MultiPolygon> ft : thisPolygons) {
						if (projE.contains(ft.getNumericField<lapis::coord_t>("X"), ft.getNumericField<lapis::coord_t>("Y"))) {
							out.addFeature(ft);
						}
					}
				}
			}
		}
		return out;
	}

	lapis::VectorDataset<lapis::MultiPolygon> LapisFolder::allPolygons() const {
		lapis::VectorDataset<lapis::MultiPolygon> out{};
		auto ntile = nTiles();
		std::optional<fs::path> file;
		for (size_t cell = 0; cell < ntile; ++cell) {
			file = mcGaugheyPolygons(cell);
			if (file) {
				if (out.nFeature()) {
					out.appendFile(file.value());
				}
				else {
					out = lapis::VectorDataset<lapis::MultiPolygon>(file.value());
				}
			}
		}
		return out;
	}

	lapis::VectorDataset<lapis::MultiPolygon> LapisFolder::polygons(const lapis::Extent& e) const {
		return mcGaugheyPolygons(e);
	}

	std::optional<fs::path> LapisFolder::polygons(size_t index) const {
		return mcGaugheyPolygons(index);
	}

	template<class T>
	std::optional<lapis::Raster<T>> fineDataByExtentGeneric(const lapis::Extent& e, const lapis::Raster<bool>& tileLayout, std::function<std::optional<fs::path>(size_t)> byTile) {
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


	std::optional<fs::path> LapisFolder::watershedSegmentRaster(size_t index) const
	{
		if (index < 0 || index >= (size_t)_layoutRaster.ncell()) {
			return std::nullopt;
		}

		auto checkFolder = [&](const fs::path& folder)->std::optional<fs::path> {
			fs::path candidate = constructFullPathTif(folder, name(), tileNameFromTile(index), "Segments", "");
			if (fs::exists(candidate)) {
				return candidate;
			}
			return std::nullopt;
			};
		std::optional<fs::path> candidate = checkFolder(_folder / "TreeApproximateObjects");
		if (candidate) {
			return candidate;
		}
		candidate = checkFolder(_folder / "TreeApproximateObjects" / "SegmentRasters");
		if (candidate) {
			return candidate;
		}
		candidate = checkFolder(_folder / "TreeApproximateObjects" / "Watershed" / "SegmentRasters");
		if (candidate) {
			return candidate;
		}
		return std::nullopt;
	}

	std::optional<fs::path> LapisFolder::watershedSegmentRaster(lapis::rowcol_t row, lapis::rowcol_t col) const
	{
		if (row < 0 || row >= _layoutRaster.nrow() || col < 0 || col >= _layoutRaster.ncol()) {
			return std::nullopt;
		}
		return watershedSegmentRaster(_layoutRaster.cellFromRowColUnsafe(row, col));
	}

	std::optional<lapis::Raster<lapis::taoid_t>> LapisFolder::watershedSegmentRaster(const lapis::Extent& e) const {
		return fineDataByExtentGeneric<lapis::taoid_t>(e, _layoutRaster, [&](size_t n) { return watershedSegmentRaster(n); });
	}

	std::optional<fs::path> LapisFolder::intensityRaster(size_t index) const
	{
		if (index < 0 || index >= (size_t)_layoutRaster.ncell()) {
			return std::nullopt;
		}
		fs::path candidate = constructFullPathTif(dir() / "Intensity", name(), tileNameFromTile(index), "MeanCanopyIntensity", "");
		if (fs::exists(candidate)) {
			return candidate;
		}
		candidate = constructFullPathTif(dir() / "Intensity", name(), tileNameFromTile(index), "MeanIntensity", "");
		if (fs::exists(candidate)) {
			return candidate;
		}
		return std::nullopt;
	}

	std::optional<fs::path> LapisFolder::intensityRaster(lapis::rowcol_t row, lapis::rowcol_t col) const
	{
		if (row < 0 || row >= _layoutRaster.nrow() || col < 0 || col >= _layoutRaster.ncol()) {
			return std::nullopt;
		}
		return intensityRaster(_layoutRaster.cellFromRowColUnsafe(row, col));
	}

	std::optional<lapis::Raster<lapis::intensity_t>> LapisFolder::intensityRaster(const lapis::Extent& e) const {
		return fineDataByExtentGeneric<lapis::intensity_t>(e, _layoutRaster, [&](size_t n) { return intensityRaster(n); });
	}

	std::optional<fs::path> LapisFolder::maxHeightRaster(size_t index) const
	{
		if (index < 0 || index >= (size_t)_layoutRaster.ncell()) {
			return std::nullopt;
		}
		auto checkFolder = [&](const fs::path& folder)->std::optional<fs::path> {
			std::optional<fs::path> candidate = checkAllUnits(folder, name(), tileNameFromTile(index), "MaxHeight");
			if (candidate) {
				return candidate;
			}
			candidate = checkAllUnits(folder, name(), tileNameFromTile(index), "TaoHeight");
			if (candidate) {
				return candidate;
			}
			return std::nullopt;
			};
		std::optional<fs::path> candidate = checkFolder(_folder / "TreeApproximateObjects");
		if (candidate) {
			return candidate;
		}
		candidate = checkFolder(_folder / "TreeApproximateObjects" / "MaxHeightRasters");
		if (candidate) {
			return candidate;
		}
		candidate = checkFolder(_folder / "TreeApproximateObjects" / "Watershed" / "TaoHeightRasters");
		if (candidate) {
			return candidate;
		}
		return std::nullopt;
	}

	std::optional<fs::path> LapisFolder::maxHeightRaster(lapis::rowcol_t row, lapis::rowcol_t col) const
	{
		if (row < 0 || row >= _layoutRaster.nrow() || col < 0 || col >= _layoutRaster.ncol()) {
			return std::nullopt;
		}
		return maxHeightRaster(_layoutRaster.cellFromRowColUnsafe(row, col));
	}

	std::optional<lapis::Raster<lapis::csm_t>> LapisFolder::maxHeightRaster(const lapis::Extent& e) const {
		return fineDataByExtentGeneric<lapis::csm_t>(e, _layoutRaster, [&](size_t n) { return maxHeightRaster(n); });
	}


	std::optional<fs::path> LapisFolder::csmRaster(size_t index) const
	{
		if (index < 0 || index >= (size_t)_layoutRaster.ncell()) {
			return std::optional<fs::path>();
		}
		return checkAllUnits(_folder / "CanopySurfaceModel", name(), tileNameFromTile(index), "CanopySurfaceModel");
	}

	std::optional<fs::path> LapisFolder::csmRaster(lapis::rowcol_t row, lapis::rowcol_t col) const
	{
		if (row < 0 || row >= _layoutRaster.nrow() || col < 0 || col >= _layoutRaster.ncol()) {
			return std::optional<fs::path>();
		}
		return csmRaster(_layoutRaster.cellFromRowColUnsafe(row, col));
	}

	std::optional<lapis::Raster<lapis::csm_t>> LapisFolder::csmRaster(const lapis::Extent& e) const {
		return fineDataByExtentGeneric<lapis::csm_t>(e, _layoutRaster, [&](size_t n) { return csmRaster(n); });
	}

	std::optional<fs::path> LapisFolder::_getMetricByName(const std::string& name, bool preferAllReturns) const
	{
		std::vector<std::string> possibleUnits = { "", "_Meters", "_Feet", "_Percent", "_Radians", "_Degrees" };
		std::vector<fs::path> folders = { _folder / "PointMetrics", _folder / "Topography", _folder / "CanopyMetrics" };
		if (preferAllReturns) {
			folders.push_back(_folder / "PointMetrics" / "AllReturns");
		}
		else {
			folders.push_back(_folder / "PointMetrics" / "FirstReturns");
		}
		std::string runNameWithUnderscore = _name.size() ? (_name + "_") : "";
		for (const fs::path& folder : folders) {
			for (const std::string& unit : possibleUnits) {
				fs::path candidate = _folder / folder / (runNameWithUnderscore + name + unit + ".tif");
				if (fs::exists(candidate)) {
					return candidate;
				}
			}
		}
		return std::optional<fs::path>();
	}

	std::function<lapis::CoordXY(const lapis::ConstFeature<lapis::MultiPolygon>&)> LapisFolder::coordGetter() const {
		return [](const lapis::ConstFeature<lapis::MultiPolygon>& ft)->lapis::CoordXY {
			return { ft.getNumericField<lapis::coord_t>("X"), ft.getNumericField<lapis::coord_t>("Y") };
			};
	}
	
	std::function<lapis::coord_t(const lapis::ConstFeature<lapis::MultiPolygon>&)> LapisFolder::heightGetter() const {
		return [](const lapis::ConstFeature<lapis::MultiPolygon>& ft)->lapis::coord_t {
			return ft.getNumericField<lapis::coord_t>("Height");
			};
	}
	
	std::function<lapis::coord_t(const lapis::ConstFeature<lapis::MultiPolygon>&)> LapisFolder::radiusGetter() const {
		return [](const lapis::ConstFeature<lapis::MultiPolygon>& ft)->lapis::coord_t {
			return std::sqrt(ft.getGeometry().area() / M_PI);
			};
	}
	
	std::function<lapis::coord_t(const lapis::ConstFeature<lapis::MultiPolygon>&)> LapisFolder::areaGetter() const {
		return [](const lapis::ConstFeature<lapis::MultiPolygon>& ft)->lapis::coord_t {
			return ft.getGeometry().area();
			};
	}

	bool isLapisFolder(const fs::path& path)
	{
		if (!fs::is_directory(path)) {
			return false;
		}
		if (!fs::exists(path / "RunParameters" / "FullParameters.ini")) {
			return false;
		}
		if (!fs::exists(path / "Layout" / "TileLayout.shp")) {
			return false;
		}
		return true;
	}

	std::optional<LapisFolder> getRunIfLapis(const fs::path& path)
	{
		if (isLapisFolder(path)) {
			return LapisFolder(path);
		}
		return std::optional<LapisFolder>();
	}

}