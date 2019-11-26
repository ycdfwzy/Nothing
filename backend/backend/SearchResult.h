#pragma once
#include "utils.h"
#include <string>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <vector>
#include <Windows.h>

namespace Nothing {

typedef std::wstring PREDECESSOR_CONTENT;
typedef std::wstring SUCCESSOR_CONTENT;
typedef std::pair<PREDECESSOR_CONTENT, SUCCESSOR_CONTENT> CONTENT_SEARCH_RESULT;

class SearchResult {
public:
	SearchResult() {}
	SearchResult(DWORDLONG ref,
		const std::wstring& name,
		const std::wstring& path,
		const std::wstring& keyword,
		bool include_contents = false) :
			ref(ref), name(name), path(path),
			keyword(keyword),
			include_contents(include_contents) {};

	Result add_content(const PREDECESSOR_CONTENT&, const SUCCESSOR_CONTENT&);
	Result add_content(const CONTENT_SEARCH_RESULT&);

	const std::vector<CONTENT_SEARCH_RESULT>& get_content_results() const
	{
		return this->content_results;
	}

	std::wstring get_name() const {
		return this->name;
	}

	std::wstring get_path() const {
		return this->path;
	}

	DWORDLONG get_reference() const {
		return this->ref;
	}

private:
	bool include_contents;

	DWORDLONG ref;
	std::wstring name;
	std::wstring path;
	std::wstring keyword;

	std::vector<CONTENT_SEARCH_RESULT> content_results;
};

} // namespace Nothing
