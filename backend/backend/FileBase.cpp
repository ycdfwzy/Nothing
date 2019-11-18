#include "FileBase.h"
#include <fstream>
#include <functional>
#include <stack>

using namespace std;
using namespace Nothing;

Result FileBase::getPath(DWORDLONG ref, std::wstring& res) const {
	if (refmap.find(ref) == refmap.end()) {
		return Result::REFERENCE_NOT_FOIUND;
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
	return Result::SUCCESS;
}

Result FileBase::save(std::wstring& path) const {
	wofstream fout(path);
	if (!fout.is_open()) {
		return Result::CANNOT_OPEN_FILE;
	}
	locale loc("zh_CN.UTF-8");
	fout.imbue(loc);
	for (const auto& p : refmap) {
		fout << p.second.name << L" " << to_wstring(p.first) << L" " << to_wstring(p.second.parent) << endl;
	}
	fout.close();
	return Result::SUCCESS;
}

void FileBase::add_file(const wstring& filename,
						DWORDLONG ref_num,
						DWORDLONG par_num) {
	this->refmap[ref_num] = FileName(filename, par_num);
	/*unordered_set<WCHAR> v;
	v.clear();
	for (const WCHAR& c : filename) {
		if (v.find(c) == v.end()) {
			this->index.insert({ c, ref_num });
			v.insert(c);
		}
	}*/
}

Result FileBase::search_by_name(const std::wstring& keyword,
							  std::vector<std::wstring>& res,
							  bool need_clear_res) const {
	if (keyword.empty()) {
		return Result::KEYWORD_EMPTY;
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
			if (this->getPath(ref, new_res) == Result::SUCCESS)
				res.emplace_back(new_res);
		}
	}
	return Result::SUCCESS;
}

Result FileBase::preprocess() {
	// $RmMetadata
	// System Volume Information
	vector<DWORDLONG> to_delete;
	// get root directory reference number
	wstring path = L"X:";
	path[0] = this->diskName;
	unordered_set<DWORDLONG> to_insert;
	to_insert.clear();
	for (const auto& p : refmap) {
		if (p.second.name == L"__MACOSX" ||
			p.second.name == L"$RmMetadata" ||
			p.second.name == L"System Volume Information") {
			to_delete.push_back(p.first);
		}
		if (refmap.find(p.second.parent) == refmap.end()) {
			// wcout << p.second.name << L" " << p.second.parent << endl;
			to_insert.insert(p.second.parent);
			if (p.second.name == L"$RmMetadata")
				to_delete.push_back(p.second.parent);
		}
	}
	for (auto& p : to_insert) {
		refmap[p] = FileName(path);
	}
	//-------------------------------------------
	// build tree
	unordered_map<DWORDLONG, DWORDLONG>* firstSon =
		new unordered_map<DWORDLONG, DWORDLONG>();
	unordered_map<DWORDLONG, DWORDLONG>* nextSibling =
		new unordered_map<DWORDLONG, DWORDLONG>();
	//stack<DWORDLONG> stk;
	firstSon->clear();
	nextSibling->clear();
	for (const auto& p : refmap) {
		if (p.second.parent == 0) {
			/*this->interval[p.first] = make_pair(this->DFSseq.size(), 0);
			this->DFSseq.push_back(p.first);
			stk.push(p.first);*/
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
	//-------------------------------------------
	this->DFSseq.clear();
	this->DFSseq.reserve(this->refmap.size());

	for (auto& p : to_insert)
		if (find(to_delete.begin(), to_delete.end(), p) == to_delete.end())
			DFS(p, false, to_delete, firstSon, nextSibling);
		else
			DFS(p, true, to_delete, firstSon, nextSibling);
	// wcout << this->refmap.size() << endl;

	delete firstSon;
	delete nextSibling;

	this->makeIndex();

	return Result::SUCCESS;
}

Result FileBase::getAllFiles(DWORDLONG root,
						   vector<DWORDLONG>& res,
						   bool no_directory) const {
	if (this->interval.find(root) == this->interval.end())
		return Result::REFERENCE_NOT_FOIUND;
	INTERVAL itv = this->interval.at(root);
	if (!no_directory) {
		res.assign(this->DFSseq.begin() + itv.first,
				   this->DFSseq.begin() + itv.second);
	}
	else
	{
		res.clear();
		res.reserve(itv.second - itv.first);
		for (int i = itv.first; i < itv.second; i++) {
			DWORDLONG ref = this->DFSseq[i];
			if (this->interval.at(ref).first + 1 >= this->interval.at(ref).second) {
				res.push_back(ref);
			}
		}
	}
	
	return Result::SUCCESS;
}

Result FileBase::makeIndex() {
	unordered_set<WCHAR> v;
	for (const auto& p : refmap) {
		v.clear();
		for (const WCHAR& c : p.second.name) {
			if (v.find(c) != v.end())
				continue;
			v.insert(c);
			this->index.insert(make_pair(c, p.first));
		}
	}
	return Result::SUCCESS;
}

Result FileBase::getReference(const std::wstring& path,
							DWORDLONG& ref) const {
	vector<wstring> splited_path;
	if (!this->splitPath(path, splited_path))
		return Result::PATH_INVALID;

	INTERVAL itv = make_pair(0, this->DFSseq.size());
	for (const wstring& filename : splited_path) {
		// wcout << filename << endl;
		size_t idx = itv.first;
		while (idx < itv.second) {
			DWORDLONG ref_ = this->DFSseq[idx];
			// wcout << this->refmap.at(ref_).name << endl;
			if (this->refmap.at(ref_).name == filename) {
				ref = ref_;
				itv = make_pair(interval.at(ref_).first+1, interval.at(ref_).second);
				break;
			}
			idx = interval.at(ref_).second;
		}
		if (idx >= itv.second)
			return Result::PATH_INVALID;
	}

	return Result::SUCCESS;
}

bool FileBase::splitPath(const std::wstring& path,
						 std::vector<std::wstring>& res) const {
	size_t last = 0;
	size_t p = path.find(L'\\');
	res.clear();
	while (last < path.npos) {
		wstring filename = (p == path.npos ?
			path.substr(last) : path.substr(last, p - last));
		if (filename.empty())
			break;
		res.push_back(filename);
		last = (p == path.npos ? p : p + 1);
		p = path.find(L'\\', last);
	}
	return true;
}

void FileBase::DFS(DWORDLONG cur, bool need_deleting, const vector<DWORDLONG>& to_delete,
				   unordered_map<DWORDLONG, DWORDLONG>* firstSon,
				   unordered_map<DWORDLONG, DWORDLONG>* nextSibling) {
	size_t s, e;
	if (!need_deleting) {
		s = this->DFSseq.size();
		this->DFSseq.push_back(cur);
	}
	else {
		this->refmap.erase(cur);
	}

	auto p = firstSon->find(cur);
	if (p != firstSon->end()) {
		if (find(to_delete.begin(), to_delete.end(), p->second) == to_delete.end())
			DFS(p->second, need_deleting, to_delete, firstSon, nextSibling);
		else
			DFS(p->second, true, to_delete, firstSon, nextSibling);
	}

	if (!need_deleting) {
		e = this->DFSseq.size();
		this->interval[cur] = make_pair(s, e);
	}

	auto q = nextSibling->find(cur);
	if (q != nextSibling->end()) {
		if (find(to_delete.begin(), to_delete.end(), q->second) == to_delete.end())
			DFS(q->second, need_deleting, to_delete, firstSon, nextSibling);
		else
			DFS(q->second, true, to_delete, firstSon, nextSibling);
	}
}
