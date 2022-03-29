#pragma once

#include "common.h"
#include "line.h"
#include "data.h"
#include "elevationSymbol.h"
#include "block.h"

class ElevationSymbolSet
{
public:
	void setMFCPaintTestVec(std::vector<int>& temp_idx, 
							std::vector<Line>& temp_lines);
	void setAnalysisDependencise(std::shared_ptr<Data> m_data);

	void parse(std::vector<std::shared_ptr<ElevationSymbol>>& _elevationSymbols,
						  std::vector<std::shared_ptr<Block>> &_blocks);

	void sortElevation(std::vector<std::shared_ptr<ElevationSymbol>>& _elevationSymbols);

	bool parseBlock(std::vector<std::shared_ptr<ElevationSymbol>>& elevationSymbols,
					std::shared_ptr<Block>& block);

	static bool bValidBlock(std::shared_ptr<Block> &block);
	

	int getClosestElevationSymbol(double yTick,
								  std::vector<std::shared_ptr<ElevationSymbol>> &elevationSymbols);

	std::shared_ptr<Data> data;
	std::vector<int>* pIdxVec;
	std::vector<Line>* pLineVec;

};
