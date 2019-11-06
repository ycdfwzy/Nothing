#include "FileBase.h"
#include <fstream>

using namespace std;

bool FileBase::getPath(DWORDLONG ref, std::wstring& res) const {
	if (ref2name.find(ref) == ref2name.end()) {
		return false;
	}
	res = ref2name.at(ref);
	ref = parent.at(ref);
	while (ref2name.find(ref) != ref2name.end()) {
		res = ref2name.at(ref) + L"\\" + res;
		ref = parent.at(ref);
	}
	res = L"X:\\" + res;
	res[0] = diskName;
	return true;
}

bool FileBase::save(std::wstring& path) const {
	wofstream fout(path);
	if (!fout.is_open()) {
		return false;
	}
	locale loc("zh_CN.UTF-8");
	fout.imbue(loc);
	for (const auto& p : ref2name) {
		fout << p.second << L" " << to_wstring(p.first) << L" " << to_wstring(parent.at(p.first)) << endl;
	}
	fout.close();
	return true;
}

void FileBase::add_file(const wstring& filename,
						DWORDLONG ref_num,
						DWORDLONG par_num) {
	this->ref2name[ref_num] = filename;
	this->parent[ref_num] = par_num;
	unordered_set<WCHAR> v;
	v.clear();
	for (const WCHAR& c : filename) {
		if (v.find(c) == v.end()) {
			this->index.insert({ c, ref_num });
			v.insert(c);
		}
	}
}

bool FileBase::search_by_name(const std::wstring& keyword,
							  std::vector<std::wstring>& res,
							  bool need_clear_res) const {
	if (keyword.empty()) {
		return false;
	}
	if (need_clear_res) {
		res.clear();
	}
	auto p = index.equal_range(keyword.at(0));
	for (auto I = p.first; I != p.second; I++) {
		DWORDLONG ref = (*I).second;
		wstring filename = ref2name.at(ref);
		if (filename.find(keyword) != filename.npos) {
			wstring new_res;
			if (this->getPath(ref, new_res))
				res.emplace_back(new_res);
		}
	}
	return true;
}