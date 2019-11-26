#include "SearchResult.h"


using namespace std;
using namespace Nothing;

Result SearchResult::add_content(const PREDECESSOR_CONTENT& pred_cont,
								const SUCCESSOR_CONTENT& succ_cont) {
	if (this->include_contents) {
		this->content_results.emplace_back(pred_cont, succ_cont);
		return Result::SUCCESS;
	}
	return Result::UNKNOWN_FAILED;
}

Result Nothing::SearchResult::add_content(const CONTENT_SEARCH_RESULT& csr) {
	if (this->include_contents) {
		this->content_results.emplace_back(csr);
		return Result::SUCCESS;
	}
	return Result::UNKNOWN_FAILED;
}
