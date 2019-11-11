#include "FileBase.h"
#include <fstream>
#include <stack>

using namespace std;

bool FileBase::getPath(DWORDLONG ref, std::wstring& res) const {
	if (refmap.find(ref) == refmap.end()) {
		return false;
	}
	res = refmap.at(ref).name;
	ref = refmap.at(ref).parent;
	while (ref != 0 && refmap.find(ref) != refmap.end()) {
		res = refmap.at(ref).name + L"\\" + res;
		ref = refmap.at(ref).parent;
	}
	if (ref != 0) {
		res = L"X:\\" + res;
		res[0] = diskName;
	}
	return true;
}

bool FileBase::save(std::wstring& path) const {
	wofstream fout(path);
	if (!fout.is_open()) {
		return false;
	}
	locale loc("zh_CN.UTF-8");
	fout.imbue(loc);
	for (const auto& p : refmap) {
		fout << p.second.name << L" " << to_wstring(p.first) << L" " << to_wstring(p.second.parent) << endl;
	}
	fout.close();
	return true;
}

void FileBase::add_file(const wstring& filename,
						DWORDLONG ref_num,
						DWORDLONG par_num) {
	this->refmap[ref_num] = FileName(filename, par_num);
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
		wstring filename = refmap.at(ref).name;
		if (filename.find(keyword) != filename.npos) {
			wstring new_res;
			if (this->getPath(ref, new_res))
				res.emplace_back(new_res);
		}
	}
	return true;
}

bool FileBase::doDFS() {
	// get root directory reference number
	wstring path = L"X:\\";
	path[0] = this->diskName;
	for (const auto& p : refmap) {
		if (refmap.find(p.second.parent) == refmap.end()) {
			//wcout << p.second.parent << endl;
			refmap[p.second.parent] = FileName(path, 0);
		}
	}
	//-------------------------------------------
	// build tree
	unordered_map<DWORDLONG, DWORDLONG>* firstSon = 
		new unordered_map<DWORDLONG, DWORDLONG>();
	unordered_map<DWORDLONG, DWORDLONG>* nextSibling =
		new unordered_map<DWORDLONG, DWORDLONG>();
	stack<DWORDLONG> stk;
	firstSon->clear();
	nextSibling->clear();
	for (const auto& p : refmap) {
		if (p.second.parent == 0) {
			this->interval[p.first] = make_pair(this->DFSseq.size(), 0);
			this->DFSseq.push_back(p.first);
			stk.push(p.first);
			continue;
		}
		if (firstSon->find(p.second.parent) == firstSon->end()) {
			firstSon->insert(make_pair(p.second.parent, p.first));
		}
		else
		{
			nextSibling->insert(make_pair(p.first, firstSon->at(p.second.parent)));
			firstSon->erase(p.second.parent);
			firstSon->insert(make_pair(p.second.parent, p.first));
		}
	}
	this->DFSseq.clear();
	this->DFSseq.reserve(this->refmap.size());
	//-------------------------------------------
	// dfs without Recursion
	while (!stk.empty()) {
		DWORDLONG cur = stk.top();
		auto p = firstSon->find(cur);
		if (p != firstSon->end()) {
			this->interval[p->second] = make_pair(this->DFSseq.size(), 0);
			this->DFSseq.push_back(p->second);
			stk.push(p->second);
			firstSon->erase(cur);
			if (nextSibling->find(p->second) != nextSibling->end())
				firstSon->insert(make_pair(cur, nextSibling->at(p->second)));
		}
		else
		{
			size_t s = this->interval[cur].first;
			this->interval[cur] = make_pair(s, this->DFSseq.size());
			stk.pop();
		}
	}
	// wcout << this->DFSseq.size() << endl;

	delete firstSon;
	delete nextSibling;

	return false;
}

bool FileBase::getAllFiles(DWORDLONG root,
						   vector<DWORDLONG>& res) const {
	if (this->interval.find(root) == this->interval.end())
		return false;
	INTERVAL itv = this->interval.at(root);
	res.assign(this->DFSseq.begin() + itv.first,
			   this->DFSseq.begin() + itv.second);
	return true;
}
