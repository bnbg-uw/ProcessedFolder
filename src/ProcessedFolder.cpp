#include"ProcessedFolder.hpp"

#include<readdbf/dbf.hpp>
#include<raster/raster.hpp>

#include<filesystem>
#include<regex>
#include<queue>

namespace lidar {

	namespace fs = std::filesystem;

	std::string stringOrThrow(std::optional<std::string> check) {
		if (check.has_value())
			return check.value();
		else {
			std::cerr << "Requested file does not exist\n";
			throw FileNotFoundException();
		}
	}

	class FusionFolder : public ProcessedFolder {
	public:

		FusionFolder(const std::string& folder);

		fs::path getDir() const override;

		std::optional<std::string> getCover() const override;
		std::optional<std::string> getP95() const override;
		std::optional<std::string> getRumple() const override;
		std::optional<std::string> getP25() const override;
		std::optional<std::string> getMeanHeight() const override;
		std::optional<std::string> getStdDevHeight() const override;

		std::optional<std::string> getSlope(sp::coord_t radius) const override;
		std::optional<std::string> getAspect(sp::coord_t radius) const override;
		std::optional<std::string> getTPI(sp::coord_t radius) const override;

		std::optional<std::string> getMaskRaster() const override;
		std::optional<std::string> getTileLayoutVector() const override;
		int nTiles() const override;
		const sp::CoordRef& getProjection() const override;
		sp::coord_t getConvFactor() const override;
		std::optional<sp::Alignment> getMetricAlignment() const override;
		std::optional<sp::Extent> extentByTile(int index) const override;

		lico::TaoList readAllHighPoints(int nThreads, double fixedRadius) const override;
		lico::TaoList getHighPointsByTile(int i, double fixedRadius = 0) const override;

		std::optional<std::string> getSegmentRaster(int index) const override;
		std::optional<std::string> getCanopyIntensityRaster(int index) const override;
		std::optional<std::string> getMaxHeightRaster(int index) const override;
		std::optional<std::string> getCsmRaster(int index) const override;

	private:
		fs::path _folder;
		fs::path _layoutPath;
		sp::SpVectorDataset<sp::SpPolygon> _layout;
		sp::CoordRef _proj;

		std::optional<std::string> _getMetric(const std::string& basename, const std::string& folderBaseName) const;
		std::optional<std::string> _getTopoMetric(const std::string& basename, sp::coord_t radiusMeters) const;
		std::optional<std::string> _getTileMetric(const std::string& basename, int index) const;
	};

	class LapisFolder : public ProcessedFolder {
	public:

		LapisFolder(const std::string& folder);

		fs::path getDir() const override;

		std::optional<std::string> getCover() const override;
		std::optional<std::string> getP95() const override;
		std::optional<std::string> getRumple() const override;
		std::optional<std::string> getP25() const override;
		std::optional<std::string> getMeanHeight() const override;
		std::optional<std::string> getStdDevHeight() const override;

		std::optional<std::string> getSlope(sp::coord_t radius) const override;
		std::optional<std::string> getAspect(sp::coord_t radius) const override;
		std::optional<std::string> getTPI(sp::coord_t radius) const override;

		std::optional<std::string> getMaskRaster() const override;
		std::optional<std::string> getTileLayoutVector() const override;
		int nTiles() const override;
		const sp::CoordRef& getProjection() const override;
		sp::coord_t getConvFactor() const override;
		std::optional<sp::Alignment> getMetricAlignment() const override;
		std::optional<sp::Extent> extentByTile(int index) const override;

		lico::TaoList readAllHighPoints(int nThreads, double fixedRadius) const override;
		lico::TaoList getHighPointsByTile(int i, double fixedRadius = 0) const override;

		std::optional<std::string> getSegmentRaster(int index) const override;
		std::optional<std::string> getCanopyIntensityRaster(int index) const override;
		std::optional<std::string> getMaxHeightRaster(int index) const override;
		std::optional<std::string> getCsmRaster(int index) const override;

	private:
		fs::path _folder;
		sp::SpVectorDataset<sp::SpPolygon> _layout;
		sp::CoordRef _proj;
		std::string _name;
		std::string _units;

		std::optional<std::string> _getPointMetric(const std::string& baseName, const std::string& units) const;
	};

	//This is just for rxgaming so it doesn't support most standard gridmetrics style layers
	class LidRFolder : public ProcessedFolder {
	public:

		LidRFolder(const std::string& folder);

		fs::path getDir() const override;

		std::optional<std::string> getCover() const override;
		std::optional<std::string> getP95() const override;
		std::optional<std::string> getRumple() const override;
		std::optional<std::string> getP25() const override;
		std::optional<std::string> getMeanHeight() const override;
		std::optional<std::string> getStdDevHeight() const override;

		std::optional<std::string> getSlope(sp::coord_t radiusMeters) const override;
		std::optional<std::string> getAspect(sp::coord_t radiusMeters) const override;
		std::optional<std::string> getTPI(sp::coord_t radiusMeters) const override;


		std::optional<std::string> getMaskRaster() const override;
		std::optional<std::string> getTileLayoutVector() const override;
		int nTiles() const override;
		const sp::CoordRef& getProjection() const override;
		sp::coord_t getConvFactor() const override;
		std::optional<sp::Alignment> getMetricAlignment() const override;
		std::optional<sp::Extent> extentByTile(int index) const override;

		lico::TaoList readAllHighPoints(int nThreads, double fixedRadius = 0) const override;
		lico::TaoList getHighPointsByTile(int i, double fixedRadius = 0) const override;

		std::optional<std::string> getSegmentRaster(int index) const override;
		std::optional<std::string> getCanopyIntensityRaster(int index) const override;
		std::optional<std::string> getTopsRaster(int index) const;
		std::optional<std::string> getMaxHeightRaster(int index) const override;
		std::optional<std::string> getCsmRaster(int index) const override;

	private:
		fs::path _folder;
		sp::SpVectorDataset<sp::SpMultiPolygon> _layout;
		sp::CoordRef _proj;
		std::string _name;
		std::string _units;

		std::optional<std::string> _getPointMetric(const std::string& baseName, const std::string& units) const;
	};

	std::unique_ptr<ProcessedFolder> getProcessedFolderReader(const std::string& folder)
	{
		try {
			std::unique_ptr<ProcessedFolder> out = std::make_unique<FusionFolder>(folder);
			return out;
		}
		catch (std::invalid_argument e) { std::cerr << "not a valid fusion folder arg\n"; }
		catch (sp::InvalidVectorFileException e) { std::cerr << "not a valid vector file - fusion\n"; }

		try {
			std::unique_ptr<ProcessedFolder> out = std::make_unique<LapisFolder>(folder);
			return out;
		}
		catch (std::invalid_argument e) { std::cerr << "cannot interpret this as a lapis folder\n"; }
		catch (sp::InvalidVectorFileException e) { std::cerr << "not a valid vector file - lapis\n"; }

		try {
			std::unique_ptr<ProcessedFolder> out = std::make_unique<LidRFolder>(folder);
			return out;
		}
		catch (std::invalid_argument e) { std::cerr << "cannot interpret this as a LidR folder\n"; }
		catch (sp::InvalidVectorFileException e) { std::cerr << "not a valid vector file - lidr\n"; }
		return std::unique_ptr<ProcessedFolder>();
	}

	FusionFolder::FusionFolder(const std::string& folder)
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
							_layout = sp::SpVectorDataset<sp::SpPolygon>(_layoutPath.string());
							break;
						}
					}
					_proj = spatial::Raster<int>(getMaskRaster().value()).projection();
					_layout.projection(_proj);
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

	fs::path FusionFolder::getDir() const
	{
		return _folder;
	}

	std::optional<std::string> FusionFolder::getCover() const
	{
		return _getMetric("all_cover_aboveUNITS", "Metrics");
	}
	std::optional<std::string> FusionFolder::getP95() const
	{
		return _getMetric("elev_P95_UNITSplus", "Metrics");
	}
	std::optional<std::string> FusionFolder::getRumple() const
	{
		return _getMetric("canopy_rumple", "CanopyMetrics");
	}
	std::optional<std::string> FusionFolder::getP25() const
	{
		return _getMetric("elev_P25_UNITSplus", "Metrics");
	}
	std::optional<std::string> FusionFolder::getMeanHeight() const
	{
		return _getMetric("elev_ave_UNITSplus", "Metrics");
	}
	std::optional<std::string> FusionFolder::getStdDevHeight() const
	{
		return _getMetric("elev_stddev_UNITSplus", "Metrics");
	}
	std::optional<std::string> FusionFolder::getSlope(sp::coord_t radius) const
	{
		return _getTopoMetric("slope", radius);
	}
	std::optional<std::string> FusionFolder::getAspect(sp::coord_t radius) const
	{
		return _getTopoMetric("aspect", radius);
	}
	std::optional<std::string> FusionFolder::getTPI(sp::coord_t radius) const
	{
		return _getTopoMetric("tpi", radius);
	}
	std::optional<std::string> FusionFolder::getMaskRaster() const
	{
		std::cout << "A";
		return _getMetric("all_cnt", "Metrics");
	}
	std::optional<std::string> FusionFolder::getTileLayoutVector() const
	{
		if (fs::exists(_layoutPath)) {
			return _layoutPath.string();
		}
		return std::optional<std::string>();
	}
	int FusionFolder::nTiles() const
	{
		return _layout.nFeatures();
	}
	const sp::CoordRef& FusionFolder::getProjection() const
	{
		return _proj;
	}
	sp::coord_t FusionFolder::getConvFactor() const
	{
		const std::optional<sp::LinearUnit>& u = _layout.projection().getXYLinearUnits();
		return sp::linearUnitPresets::meter.convertOneToThis(1, u);
	}
	std::optional<sp::Alignment> FusionFolder::getMetricAlignment() const
	{
		std::optional<std::string> maskFile = getMaskRaster();
		if (maskFile.has_value()) {
			return sp::Alignment(maskFile.value());
		}
		return std::optional<sp::Alignment>();
	}
	std::optional<sp::Extent> FusionFolder::extentByTile(int index) const
	{
		if (index < 0 || index >= nTiles()) {
			return std::optional<sp::Extent>();
		}
		return _layout.getFeature(index).geom;
	}

	lico::TaoList FusionFolder::readAllHighPoints(int nThreads, double fixedRadius) const
	{
		std::vector<lico::index_t> indexByTileN;
		auto idVec = this->_layout.getColumn("Identifier");

		fs::path segments = _folder / "Segments_2p4606FEET";
		if (!fs::exists(segments)) {
			segments = _folder / "Segments_0p75METERS";
		}
		if (!fs::exists(segments)) {
			return lico::TaoList();
		}

		lico::index_t taocount = 0;
		for (int i = 0; i < idVec.size(); ++i) {
			std::string identifier = idVec[i];
			fs::path thissegments = segments / (identifier + "_segments_Polygons.dbf");
			std::filebuf fb;
			if (!fb.open(thissegments.string(), std::ios::in)) {
				indexByTileN.push_back(-1);
				continue;
			}
			fb.close();
			indexByTileN.push_back(taocount);
			taocount += dbf::nRecords(thissegments.string());
		}

		std::vector<spatial::coord_t> xvec; xvec.resize(taocount);
		std::vector<spatial::coord_t> yvec; yvec.resize(taocount);
		std::vector<lico::unit_t> hvec; hvec.resize(taocount);
		std::vector<lico::unit_t> crownvec; crownvec.resize(taocount);
		std::vector<lico::unit_t> areavec; areavec.resize(taocount);

		int sofar = 0;
		std::mutex mut;

		auto threadFunc = [&]() {
			while (sofar < nTiles()) {
				mut.lock();
				int thistile = sofar;
				++sofar;
				mut.unlock();
				if (thistile >= indexByTileN.size()) {
					break;
				}
				if (indexByTileN[thistile] < 0) {
					continue;
				}

				std::string identifier = idVec[thistile];
				fs::path thissegments = segments / (identifier + "_segments_Polygons.dbf");
				std::filebuf fb;
				if (!fb.open(thissegments.string(), std::ios::in)) {
					continue;
				}
				fb.close();
				dbf::Dbf d{ thissegments.string() };

				std::vector<std::string> colNames = d.fieldNames();
				int xidx = -1; std::regex xr{ ".*HighX.*" };
				int yidx = -1; std::regex yr{ ".*HighY.*" };
				int aidx = -1; std::regex ar{ ".*Area.*" };
				int hidx = -1; std::regex hr{ ".*MaxHt.*" };
				for (int i = 0; i < colNames.size(); ++i) {
					if (std::regex_match(colNames[i], xr)) {
						xidx = i;
					}
					else if (std::regex_match(colNames[i], yr)) {
						yidx = i;
					}
					else if (std::regex_match(colNames[i], ar)) {
						aidx = i;
					}
					else if (std::regex_match(colNames[i], hr)) {
						hidx = i;
					}
				}
				if (xidx == -1 || yidx == -1 || aidx == -1 || hidx == -1) {
					continue;
				}
				spatial::Extent e = extentByTile(thistile).value();

				auto n = d.nRecords();

				for (lico::index_t t = 0; t < n; ++t) {
					lico::index_t thisidx = t + indexByTileN[thistile];
					spatial::coord_t x = d.getRecordAsDouble(xidx, t);
					spatial::coord_t y = d.getRecordAsDouble(yidx, t);
					if (!e.contains(x, y)) {
						xvec[thisidx] = std::numeric_limits<double>::min(); //a marker that this tao should be skipped at the very end
						continue;
					}
					xvec[thisidx] = x;
					yvec[thisidx] = y;
					hvec[thisidx] = d.getRecordAsDouble(hidx, t);
					areavec[thisidx] = d.getRecordAsDouble(aidx, t);
					if (fixedRadius > 0) {
						crownvec[thisidx] = fixedRadius;
					}
					else {
						crownvec[thisidx] = std::sqrt(areavec[thisidx] / M_PI);
					}

				}
			}
		};

		std::vector<std::thread> threads;
		for (int i = 0; i < nThreads; ++i) {
			threads.push_back(std::thread(threadFunc));
		}
		for (int i = 0; i < nThreads; ++i) {
			threads[i].join();
		}
		//copy all the data to the final taolist, skipping the ones in the buffer
		lico::TaoList tl;
		tl.reserve(taocount);
		for (lico::index_t t = 0; t < xvec.size(); ++t) {
			auto x = xvec[t];
			if (x == std::numeric_limits<double>::min()) {
				continue;
			}
			tl._xvec.push_back(x);
			tl._yvec.push_back(yvec[t]);
			tl._hvec.push_back(hvec[t]);
			tl._areavec.push_back(areavec[t]);
			tl._crownvec.push_back(crownvec[t]);
		}

		return tl;
	}

	lico::TaoList FusionFolder::getHighPointsByTile(int i, double fixedRadius) const {
		fs::path segments = _folder / "Segments_2p4606FEET";
		if (!fs::exists(segments)) {
			segments = _folder / "Segments_0p75METERS";
		}
		if (!fs::exists(segments)) {
			return lico::TaoList();
		}
		auto identifier = this->_layout.getColumn("Identifier")[i];
		lico::index_t taocount = 0;

		fs::path thissegments = segments / (identifier + "_segments_Polygons.dbf");
		std::filebuf fb;
		if (!fb.open(thissegments.string(), std::ios::in)) {
			return lico::TaoList();
		}
		fb.close();

		dbf::Dbf d{ thissegments.string() };
		taocount += dbf::nRecords(thissegments.string());

		std::vector<spatial::coord_t> xvec; xvec.resize(taocount);
		std::vector<spatial::coord_t> yvec; yvec.resize(taocount);
		std::vector<lico::unit_t> hvec; hvec.resize(taocount);
		std::vector<lico::unit_t> crownvec; crownvec.resize(taocount);
		std::vector<lico::unit_t> areavec; areavec.resize(taocount);


		std::vector<std::string> colNames = d.fieldNames();
		int xidx = -1; std::regex xr{ ".*HighX.*" };
		int yidx = -1; std::regex yr{ ".*HighY.*" };
		int aidx = -1; std::regex ar{ ".*Area.*" };
		int hidx = -1; std::regex hr{ ".*MaxHt.*" };
		for (int i = 0; i < colNames.size(); ++i) {
			if (std::regex_match(colNames[i], xr)) {
				xidx = i;
			}
			else if (std::regex_match(colNames[i], yr)) {
				yidx = i;
			}
			else if (std::regex_match(colNames[i], ar)) {
				aidx = i;
			}
			else if (std::regex_match(colNames[i], hr)) {
				hidx = i;
			}
		}
		if (xidx == -1 || yidx == -1 || aidx == -1 || hidx == -1) {
			return lico::TaoList();
		}

		spatial::Extent e = extentByTile(i).value();

		auto n = d.nRecords();

		for (lico::index_t t = 0; t < n; ++t) {
			spatial::coord_t x = d.getRecordAsDouble(xidx, t);
			spatial::coord_t y = d.getRecordAsDouble(yidx, t);
			if (!e.contains(x, y)) {
				xvec[t] = std::numeric_limits<double>::min(); //a marker that this tao should be skipped at the very end
				continue;
			}
			xvec[t] = x;
			yvec[t] = y;
			hvec[t] = d.getRecordAsDouble(hidx, t);
			areavec[t] = d.getRecordAsDouble(aidx, t);
			if (fixedRadius > 0) {
				crownvec[t] = fixedRadius;
			}
			else {
				crownvec[t] = std::sqrt(areavec[t] / M_PI);
			}
		}

		lico::TaoList tl;
		tl.reserve(taocount);
		for (lico::index_t t = 0; t < xvec.size(); ++t) {
			auto x = xvec[t];
			if (x == std::numeric_limits<double>::min()) {
				continue;
			}
			tl._xvec.push_back(x);
			tl._yvec.push_back(yvec[t]);
			tl._hvec.push_back(hvec[t]);
			tl._areavec.push_back(areavec[t]);
			tl._crownvec.push_back(crownvec[t]);
		}

		return tl;
	}


	std::optional<std::string> FusionFolder::getSegmentRaster(int index) const
	{
		return _getTileMetric("segments_Basin_Map.img", index);
	}
	std::optional<std::string> FusionFolder::getCanopyIntensityRaster(int index) const
	{
		return _getTileMetric("segments_INT_GE_2m_UNITS.img", index);
	}
	std::optional<std::string> FusionFolder::getMaxHeightRaster(int index) const
	{
		return _getTileMetric("segments_Max_Height_Map.img", index);
	}
	std::optional<std::string> FusionFolder::getCsmRaster(int index) const
	{
		if (index < 0 || index >= nTiles()) {
			return std::optional<std::string>();
		}
		std::string blockName = "BLOCK" + _layout.getFeature(index).getAttributeAsString("Blocknum");
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
	std::optional<std::string> FusionFolder::_getMetric(const std::string& basename, const std::string& folderBaseName) const
	{

		static std::regex units{ "UNITS" };
		std::cout << "b";
		fs::path metersFolder = folderBaseName + "_30METERS";
		std::cout << "c";

		std::string metersName = std::regex_replace(basename, units, "2") + "_30METERS.img";
		std::cout << "d";

		fs::path fullPathMeters = _folder / metersFolder / metersName;
		std::cout << "e";

		std::cout << fullPathMeters.string() << "\n";

		if (fs::exists(fullPathMeters)) {
			return fullPathMeters.string();
		}

		fs::path feetFolder = folderBaseName + "_98p424FEET";
		std::string feetName = std::regex_replace(basename, units, "6p5616") + "_98p424FEET.img";
		fs::path fullPathFeet = _folder / feetFolder / feetName;
		if (fs::exists(fullPathFeet)) {
			return fullPathFeet.string();
		}

		return std::optional<std::string>();
	}
	std::optional<std::string> FusionFolder::_getTopoMetric(const std::string& basename, sp::coord_t radiusMeters) const
	{
		std::string folderBaseName = "TopoMetrics";
		auto lookForFile = [&](const std::string& resName, sp::coord_t convFactor)->std::optional<std::string> {
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
					spatial::unit_t scalenum = std::stod(scalestring);
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
	std::optional<std::string> FusionFolder::_getTileMetric(const std::string& basename, int index) const
	{
		if (index < 0 || index >= nTiles()) {
			return std::optional<std::string>();
		}

		std::string tileName = _layout.getFeature(index).getAttributeAsString("Identifier");

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

	LapisFolder::LapisFolder(const std::string& folder)
	{
		if (!fs::exists(folder)) {
			throw std::invalid_argument("Folder does not exist");
		}
		if (!fs::exists(fs::path(folder) / "Layout")) {
			throw std::invalid_argument("Not a lapis folder");
		}

		_folder = folder;
		_layout = sp::SpVectorDataset<sp::SpPolygon>((_folder / "Layout" / "TileLayout.shp").string());

		if (!fs::exists(_folder / "PointMetrics")) {
			return;
		}
		std::regex match{"^.*Mean_CanopyHeight_(Meters|Feet)\\.tif$"};
		std::regex deleteAllButName{ "Mean_CanopyHeight_(Meters|Feet)\\.tif$" };
		std::regex deleteBeforeUnits{ "^.*Mean_CanopyHeight" };
		std::regex deleteAfterUnits{ "\\.tif$" };
		for (const auto& fn : fs::recursive_directory_iterator(_folder / "PointMetrics")) {
			std::string fileName = fn.path().filename().string();
			if (!std::regex_match(fileName, match)) {
				continue;
			}
			_name = std::regex_replace(fileName, deleteAllButName, "");
			_units = std::regex_replace(fileName, deleteBeforeUnits, "");
			_units = std::regex_replace(_units, deleteAfterUnits, "");
		}

		_proj = spatial::Raster<int>(getMaskRaster().value()).projection();
		_layout.projection(_proj);
	}

	fs::path LapisFolder::getDir() const
	{
		return _folder;
	}

	std::optional<std::string> LapisFolder::getCover() const
	{
		return _getPointMetric("CanopyCover", "_Percent");
	}
	std::optional<std::string> LapisFolder::getP95() const
	{
		return _getPointMetric("95thPercentile_CanopyHeight", _units);
	}
	std::optional<std::string> LapisFolder::getRumple() const
	{
		fs::path fullName = _folder / "CanopyMetrics" / (_name + "RumpleCsm.tif");
		if (fs::exists(fullName)) {
			return fullName.string();
		}
		return std::optional<std::string>();
	}
	std::optional<std::string> LapisFolder::getP25() const
	{
		return _getPointMetric("25thPercentile_CanopyHeight", _units);
	}
	std::optional<std::string> LapisFolder::getMeanHeight() const
	{
		return _getPointMetric("Mean_CanopyHeight", _units);
	}
	std::optional<std::string> LapisFolder::getStdDevHeight() const
	{
		return _getPointMetric("StdDev_CanopyHeight", _units);
	}
	std::optional<std::string> LapisFolder::getSlope(sp::coord_t radius) const
	{
		return std::optional<std::string>();
	}
	std::optional<std::string> LapisFolder::getAspect(sp::coord_t radius) const
	{
		fs::path out = _folder / "Topography" / (_name + "Aspect_Degrees.tif");
		if (fs::exists(out)) {
			return out.string();
		}
		return std::optional<std::string>();
	}
	std::optional<std::string> LapisFolder::getTPI(sp::coord_t radius) const
	{
		sp::coord_t convFactor = _units == "_Feet" ? 0.3048 : 1;
		fs::path topoFolder = _folder / "Topography";
		if (!fs::exists(topoFolder)) {
			return std::optional<std::string>();
		}
		std::regex deleteBefore{"^" + _name + "TopoPositionIndex_"};
		std::regex deleteAfter{ "(Meters_Meters|Feet_Feet)\\.tif$" };
		for (const auto& fn : fs::directory_iterator(topoFolder)) {
			std::string scaleName = fn.path().filename().string();
			scaleName = std::regex_replace(scaleName, deleteBefore, "");
			scaleName = std::regex_replace(scaleName, deleteAfter, "");
			try {
				sp::coord_t scale = std::stod(scaleName);
				if (std::abs(scale - (radius / convFactor)) < 1) {
					return fn.path().string();
				}
			}
			catch (std::invalid_argument e) {
				continue;
			}
		}
		return std::optional<std::string>();
	}
	std::optional<std::string> LapisFolder::getMaskRaster() const
	{
		return _getPointMetric("TotalReturnCount", "");
	}
	std::optional<std::string> LapisFolder::getTileLayoutVector() const
	{
		fs::path fullPath = _folder / "Layout" / "TileLayout.shp";
		if (fs::exists(fullPath)) {
			return fullPath.string();
		}
		return std::optional<std::string>();
	}
	int LapisFolder::nTiles() const
	{
		return _layout.nFeatures();
	}
	const sp::CoordRef& LapisFolder::getProjection() const
	{
		return _proj;
	}
	sp::coord_t LapisFolder::getConvFactor() const
	{
		const std::optional<sp::LinearUnit>& u = _layout.projection().getXYLinearUnits();
		return sp::linearUnitPresets::meter.convertOneToThis(1, u);
	}
	std::optional<sp::Alignment> LapisFolder::getMetricAlignment() const
	{
		std::optional<std::string> name = getMaskRaster();
		if (name.has_value()) {
			return sp::Alignment(name.value());
		}
		return std::optional<sp::Alignment>();
	}
	std::optional<sp::Extent> LapisFolder::extentByTile(int index) const
	{
		if (index < 0 || index >= nTiles()) {
			return std::optional<sp::Extent>();
		}
		return _layout.getFeature(index).geom;
	}

	lico::TaoList LapisFolder::readAllHighPoints(int nThreads, double fixedRadius) const
	{
		std::vector<size_t> tileStartingPoints;
		tileStartingPoints.reserve(nTiles());
		size_t nTaos = 0;
		auto nameVec = _layout.getColumnAsString("Name");
		for (int i = 0; i < nTiles(); ++i) {
			std::string tileName = _layout.getFeature(i).getAttributeAsString("Name");
			fs::path fileName = _folder / "TreeApproximateObjects" / (_name + "TAOs_" + tileName + ".dbf");
			if (!fs::exists(fileName)) {
				if (fs::exists(_folder / "TreeApproximateObjects" / "Points")) {
					fileName = _folder / "TreeApproximateObjects" / "Points" / (_name + "TAOs_" + tileName + ".dbf");
				}
			}
			std::filebuf fb;
			if (!fb.open(fileName.string(), std::ios::in)) {
				tileStartingPoints.push_back(-1);
				continue;
			}
			fb.close();
			tileStartingPoints.push_back(nTaos);
			nTaos += dbf::nRecords(fileName.string());
		}

		lico::TaoList tl;
		tl.resize(nTaos);


		int sofar = 0;
		std::mutex mut;
		auto threadFunc = [&]() {
			while (sofar < nTiles()) {
				mut.lock();
				int thistile = sofar;
				++sofar;
				mut.unlock();
				if (thistile >= tileStartingPoints.size()) {
					break;
				}
				if (tileStartingPoints[thistile] == -1) {
					continue;
				}
				std::string tileName = _layout.getFeature(thistile).getAttributeAsString("Name");
				fs::path fileName = _folder / "TreeApproximateObjects" / (_name+ "TAOs_" + tileName + ".dbf");
				if (!fs::exists(fileName)) {
					fileName = _folder / "TreeApproximateObjects" / "Points" / (_name + "TAOs_" + tileName + ".dbf");
				}
				std::cout << fileName << "\n";
				std::filebuf fb;
				if (!fb.open(fileName.string(), std::ios::in)) {
					continue;
				}
				fb.close();
				dbf::Dbf d{ fileName.string() };
				std::vector<std::string> colNames = d.fieldNames();
				int xidx = -1;
				int yidx = -1;
				int aidx = -1;
				int hidx = -1;
				int ridx = -1;
				for (int i = 0; i < colNames.size(); ++i) {
					if (colNames[i]=="X") {
						xidx = i;
					}
					else if (colNames[i]=="Y") {
						yidx = i;
					}
					else if (colNames[i]=="Area") {
						aidx = i;
					}
					else if (colNames[i]=="Height") {
						hidx = i;
					}
					else if (colNames[i] == "Radius") {
						ridx = i;
					}
				}
				if (xidx == -1 || yidx == -1 || aidx == -1 || hidx == -1 || ridx == -1) {
					throw std::runtime_error("Invalid column names in dbf");
				}


				size_t index = tileStartingPoints[thistile];
				for (size_t i = 0; i < d.nRecords(); ++i) {
					tl.x()[index] = d.getRecordAsDouble(xidx, i);
					tl.y()[index] = d.getRecordAsDouble(yidx, i);
					tl.area()[index] = d.getRecordAsDouble(aidx, i);
					tl.height()[index] = d.getRecordAsDouble(hidx, i);
					if (fixedRadius > 0) {
						tl.crown()[index] = fixedRadius;
					}
					else {
						tl.crown()[index] = d.getRecordAsDouble(ridx, i);
					}
					++index;
				}
			}
		};
		std::vector<std::thread> threads;
		for (int i = 0; i < nThreads; ++i) {
			threads.push_back(std::thread(threadFunc));
		}
		for (int i = 0; i < nThreads; ++i) {
			threads[i].join();
		}
		return tl;
	}

	lico::TaoList LapisFolder::getHighPointsByTile(int i, double fixedRadius) const
	{
		if (i < 0 || i >= nTiles()) {
			throw std::out_of_range("Invalid tile index");
		}

		std::cout << ".1";
		std::string tileName = _layout.getFeature(i).getAttributeAsString("Name");
		fs::path fileName = _folder / "TreeApproximateObjects" / (_name + "TAOs_" + tileName + ".dbf");
		if (!fs::exists(fileName)) {
			fileName = _folder / "TreeApproximateObjects" / "Points" / (_name + "TAOs_" + tileName + ".dbf");
			if (!fs::exists(fileName)) {
				throw lidar::FileNotFoundException("tao file does not exist");
			}
		}
		std::cout << ".2";
		std::cout << fileName.string() << "\n";
		dbf::Dbf d{ fileName.string() };
		std::vector<std::string> colNames = d.fieldNames();
		std::cout << ".21";
		int xidx = -1;
		int yidx = -1;
		int aidx = -1;
		int hidx = -1;
		int ridx = -1;
		for (int i = 0; i < colNames.size(); ++i) {
			if (colNames[i] == "X") {
				xidx = i;
			}
			else if (colNames[i] == "Y") {
				yidx = i;
			}
			else if (colNames[i] == "Area") {
				aidx = i;
			}
			else if (colNames[i] == "Height") {
				hidx = i;
			}
			else if (colNames[i] == "Radius") {
				ridx = i;
			}
		}
		std::cout << ".22";
		if (xidx == -1 || yidx == -1 || aidx == -1 || hidx == -1 || ridx == -1) {
			throw std::runtime_error("Invalid column names in dbf");
		}
		std::cout << ".3";

		size_t nRecords = d.nRecords();
		lico::TaoList tl;
		tl.resize(nRecords);
		for (size_t j = 0; j < nRecords; ++j) {
			tl.x()[j] = d.getRecordAsDouble(xidx, j);
			tl.y()[j] = d.getRecordAsDouble(yidx, j);
			tl.area()[j] = d.getRecordAsDouble(aidx, j);
			tl.height()[j] = d.getRecordAsDouble(hidx, j);
			if (fixedRadius > 0) {
				tl.crown()[j] = fixedRadius;
			}
			else {
				tl.crown()[j] = d.getRecordAsDouble(ridx, j);
			}
		}
		std::cout << ".4";

		return tl;
	}

	std::optional<std::string> LapisFolder::getSegmentRaster(int index) const
	{
		if (index < 0 || index >= nTiles()) {
			return std::optional<std::string>();
		}
		std::string tileName = _layout.getFeature(index).getAttributeAsString("Name");
		fs::path expected = _folder / "TreeApproximateObjects" / (_name + "Segments_" + tileName + ".tif");
		if (fs::exists(expected)) {
			return expected.string();
		}
		expected = _folder / "TreeApproximateObjects" / "SegmentRasters" / (_name + "Segments_" + tileName + ".tif");
		if (fs::exists(expected)) {
			return expected.string();
		}
		return std::optional<std::string>();
	}
	std::optional<std::string> LapisFolder::getCanopyIntensityRaster(int index) const
	{
		if (index < 0 || index >= nTiles()) {
			return std::optional<std::string>();
		}
		std::string tileName = _layout.getFeature(index).getAttributeAsString("Name");
		fs::path expectedOne = _folder / "Intensity" / (_name + "MeanCanopyIntensity_" + tileName + ".tif");
		fs::path expectedTwo = _folder / "Intensity" / (_name + "MeanIntensity_" + tileName + ".tif");
		if (fs::exists(expectedOne)) {
			return expectedOne.string();
		}
		if (fs::exists(expectedTwo)) {
			return expectedTwo.string();
		}
		return std::optional<std::string>();
	}
	std::optional<std::string> LapisFolder::getMaxHeightRaster(int index) const
	{
		if (index < 0 || index >= nTiles()) {
			return std::optional<std::string>();
		}
		std::string tileName = _layout.getFeature(index).getAttributeAsString("Name");
		std::cout << "cpprxgaming#992 " << _name << "\n";
		fs::path expected = _folder / "TreeApproximateObjects" / (_name + "MaxHeight_" + tileName + _units + ".tif");
		if (fs::exists(expected)) {
			return expected.string();
		}
		expected = _folder / "TreeApproximateObjects" / "MaxHeightRasters" / (_name + "MaxHeight_" + tileName + _units + ".tif");
		if (fs::exists(expected)) {
			return expected.string();
		}
		return std::optional<std::string>();
	}
	std::optional<std::string> LapisFolder::getCsmRaster(int index) const
	{
		if (index < 0 || index >= nTiles()) {
			return std::optional<std::string>();
		}

		std::string tileName = _layout.getFeature(index).getAttributeAsString("Name");
		fs::path expected = _folder / "CanopySurfaceModel" / (_name + "CanopySurfaceModel_" + tileName + _units + ".tif");
		if (fs::exists(expected)) {
			return expected.string();
		}
		return std::optional<std::string>();
	}
	std::optional<std::string> LapisFolder::_getPointMetric(const std::string& baseName, const std::string& units) const
	{
		fs::path pmFolder = _folder / "PointMetrics";
		if (fs::exists(pmFolder / "AllReturns")) {
			pmFolder /= "AllReturns";
		}
		fs::path fullName = pmFolder / (_name + baseName + units + ".tif");
		if (fs::exists(fullName)) {
			return fullName.string();
		}
		return std::optional<std::string>();

	}

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
		if(fs::exists(path))
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

				auto tao = lico::Tao(tops.xFromCell(c), tops.yFromCell(c), chm[c].value(), r, map[basin[c].value()]*convarea);
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

} //namespace lidar