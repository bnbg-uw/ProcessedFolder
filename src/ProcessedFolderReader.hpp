#pragma once
#ifndef PROCESSEDFOLDERREADER_H
#define PROCESSEDFOLDERREADER_H

#include "FusionFolder.hpp"
#include "LapisFolder.hpp"
#include "LidRFolder.hpp"

namespace processedfolder {
	static std::unique_ptr<ProcessedFolder> processedFolderReader(const std::string& folder)
	{
		try {
			std::unique_ptr<ProcessedFolder> out = std::make_unique<FusionFolder>(folder);
			return out;
		}
		catch (std::invalid_argument e) { std::cerr << "not a valid fusion folder arg\n"; }
		catch (lapis::InvalidVectorFileException e) { std::cerr << "not a valid vector file - fusion\n"; }

		try {
			std::unique_ptr<ProcessedFolder> out = std::make_unique<LapisFolder>(folder);
			return out;
		}
		catch (std::invalid_argument e) { std::cerr << "cannot interpret this as a lapis folder\n"; }
		catch (lapis::InvalidVectorFileException e) { std::cerr << "not a valid vector file - lapis\n"; }

		try {
			std::unique_ptr<ProcessedFolder> out = std::make_unique<LidRFolder>(folder);
			return out;
		}
		catch (std::invalid_argument e) { std::cerr << "cannot interpret this as a LidR folder\n"; }
		catch (lapis::InvalidVectorFileException e) { std::cerr << "not a valid vector file - lidr\n"; }

		return std::unique_ptr<ProcessedFolder>();
	}
} //namespace processedfolder

#endif