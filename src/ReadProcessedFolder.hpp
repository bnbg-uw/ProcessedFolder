#pragma once
#ifndef READPROCESSEDFOLDER_H
#define READPROCESSEDFOLDER_H

#include "FusionFolder.hpp"
#include "LapisFolder.hpp"
#include "LidRFolder.hpp"

namespace processedfolder {
	static std::unique_ptr<ProcessedFolder> readProcessedFolder(const std::string& folder)
	{
		try {
			std::unique_ptr<ProcessedFolder> out = std::make_unique<FusionFolder>(folder);
			return out;
		}
		catch (...) {}

		try {
			std::unique_ptr<ProcessedFolder> out = std::make_unique<LapisFolder>(folder);
			return out;
		}
		catch (...) {}

		try {
			std::unique_ptr<ProcessedFolder> out = std::make_unique<LidRFolder>(folder);
			return out;
		}
		catch (...) {}

		return std::unique_ptr<ProcessedFolder>();
	}
} //namespace processedfolder

#endif