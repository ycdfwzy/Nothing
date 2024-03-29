#include "FileBase.h"
#include <fstream>
#include <functional>
#include <stack>
#include <queue>

using namespace std;
using namespace Nothing;

Result FileBase::getPath(DWORDLONG ref, std::wstring& res) const {
	if (refmap.find(ref) == refmap.end()) {
		return Result::REFERENCE_NOT_FOIUND;
	}
	res = refmap.at(ref).name;
	ref = refmap.at(ref).parent;
	while (ref != 0 && refmap.find(ref) != refmap.end() && res.length() < MAX_PATH) {
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
}

void FileBase::add_file_watching(const std::wstring& filename,
										  DWORDLONG ref_num,
										  DWORDLONG par_num) {
	WaitForSingleObject(hMutex, INFINITE);

	if (this->refmap.find(par_num) != this->refmap.end()) {
		this->refmap[ref_num] = FileName(filename, par_num);
		// update tree
		this->insertTreeLink(ref_num, par_num);
		// update index
		this->addIndex(ref_num);
	}

	ReleaseMutex(hMutex);
}

void FileBase::delete_file_watching(DWORDLONG ref_num) {
	WaitForSingleObject(hMutex, INFINITE);

	if (refmap.find(ref_num) == refmap.end()) {
		ReleaseMutex(hMutex);
		return;
	}
	// update index
	this->removeIndex(ref_num);
	// update tree
	this->deleteTreeLink(ref_num, refmap.at(ref_num).parent);

	vector<DWORDLONG> tmp;
	BFS(ref_num, tmp);
	for (auto ref : tmp) {
		this->refmap.erase(ref);
		// this->father.erase(ref);
		this->firstSon.erase(ref);
		this->nextSibling.erase(ref);
	}

	ReleaseMutex(hMutex);
}

void FileBase::change_file_watching(const std::wstring& filename,
									DWORDLONG ref_num,
									DWORDLONG par_num) {
	WaitForSingleObject(hMutex, INFINITE);

	if (refmap.find(ref_num) == refmap.end()) {
		ReleaseMutex(hMutex);
		return;
	}
	if (refmap.at(ref_num).name != filename ||
		refmap.at(ref_num).parent != par_num) {
		bool filenameChanged = (refmap.at(ref_num).name != filename);
		if (filenameChanged) {
			this->removeIndex(ref_num);
		}

		if (refmap.at(ref_num).parent != par_num) {
			this->deleteTreeLink(ref_num, refmap.at(ref_num).parent);
			this->insertTreeLink(ref_num, par_num);
		}

		refmap[ref_num] = FileName(filename, par_num);
		if (filenameChanged) {
			this->addIndex(ref_num);
		}
	}

	ReleaseMutex(hMutex);
}

Result FileBase::search_by_name(const wstring& keyword,
							  vector<SearchResult>& res,
							  bool need_clear_res,
							  DWORDLONG master_path) const {
	WaitForSingleObject(hMutex, INFINITE);

	// wcout << keyword << endl;
	if (keyword.empty()) {
		if (master_path == 0) {
			for (const auto& p : refmap) {
				wstring tmp;
				if (getPath(p.first, tmp) == Result::SUCCESS) {
					res.emplace_back(p.first, p.second.name, tmp, keyword);
				}
			}
		}
		else {
			vector<DWORDLONG> seq;
			BFS(master_path, seq);
			for (const auto& p : seq) {
				wstring tmp;
				if (getPath(p, tmp) == Result::SUCCESS) {
					res.emplace_back(p, refmap.at(p).name, tmp, keyword);
				}
			}
		}
		ReleaseMutex(hMutex);
		return Result::KEYWORD_EMPTY;
	}
	if (need_clear_res) {
		res.clear();
	}

	wstring master_path_s;
	if (master_path != 0)
		this->getPath(master_path, master_path_s);

	// wcout << "before equal_range" << endl;
	auto p = index.equal_range(keyword.at(0));
	// wcout << "after equal_range" << endl;
	for (auto I = p.first; I != p.second; I++) {
		DWORDLONG ref = (*I).second;
		wstring filename = refmap.at(ref).name;
		// wcout << filename << endl;
		if (filename.find(keyword) != filename.npos) {
			wstring path;
			if (this->getPath(ref, path) == Result::SUCCESS &&
				(master_path == 0 || startsWith(path, master_path_s))) {
				res.emplace_back(ref, filename, path, keyword);
			}
		}
	}
	ReleaseMutex(hMutex);
	return Result::SUCCESS;
}

Result FileBase::preprocess() {
	WaitForSingleObject(hMutex, INFINITE);

	// $RmMetadata
	// System Volume Information
	vector<DWORDLONG> to_delete;
	// get root directory reference number
	wstring path = L"X:";
	path[0] = this->diskName;
	roots.clear();
	for (const auto& p : refmap) {
		// father[p.first] = p.second.parent;
		if (p.second.name == L"__MACOSX" ||
			p.second.name == L"$RmMetadata" ||
			p.second.name == L"System Volume Information") {
			to_delete.push_back(p.first);
		}
		if (refmap.find(p.second.parent) == refmap.end()) {
			// wcout << p.second.name << L" " << p.second.parent << endl;
			roots.insert(p.second.parent);
			if (p.second.name == L"$RmMetadata")
				to_delete.push_back(p.second.parent);
		}
	}
	for (auto p : to_delete) {
		if (roots.find(p) != roots.end()) {
			roots.erase(p);
		}
	}
	for (auto& p : roots) {
		refmap[p] = FileName(path);
	}
	//-------------------------------------------
	// build tree
	firstSon.clear();
	nextSibling.clear();
	for (const auto& p : refmap) {
		if (p.second.parent == 0) {
			continue;
		}
		insertTreeLink(p.first, p.second.parent);
	}
	//-------------------------------------------
	vector<DWORDLONG> tmp;
	for (auto ref : to_delete) {
		BFS(ref, tmp);
	}
	for (auto ref : tmp) {
		this->refmap.erase(ref);
	}

	this->makeIndex();

	ReleaseMutex(hMutex);
	wcout << "pre-process is over!" << endl;

	return Result::SUCCESS;
}

Result FileBase::getAllFiles(DWORDLONG root,
						   vector<DWORDLONG>& res,
						   bool no_directory) const {
	WaitForSingleObject(hMutex, INFINITE);

	res.clear();
	if (no_directory) {
		vector<DWORDLONG> tmp;
		BFS(root, tmp);
		for (auto ref : tmp) {
			wstring path;
			if (this->getPath(ref, path) == Result::SUCCESS) {
				// check isn't directory
				WIN32_FIND_DATAW FindFileData;
				FindClose(FindFirstFileW(path.c_str(), &FindFileData));
				if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					res.push_back(ref);
				}
			}
		}
	}
	else {
		BFS(root, res);
	}

	ReleaseMutex(hMutex);
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
	/*wcout << this->index.size() << endl;
	wcout << this->index.max_size() << endl;*/
	return Result::SUCCESS;
}

void FileBase::addIndex(DWORDLONG ref_num) {
	if (this->refmap.find(ref_num) == this->refmap.end())
		return;
	unordered_set<WCHAR> v;
	for (const wchar_t& c : this->refmap.at(ref_num).name) {
		if (v.find(c) != v.end())
			continue;
		v.insert(c);
		this->index.insert(make_pair(c, ref_num));
	}
}

void FileBase::removeIndex(DWORDLONG ref_num) {
	if (this->refmap.find(ref_num) == this->refmap.end())
		return;
	unordered_set<WCHAR> v;
	for (const wchar_t& c : this->refmap.at(ref_num).name) {
		if (v.find(c) != v.end())
			continue;
		v.insert(c);
		auto p = index.equal_range(c);
		for (auto I = p.first; I != p.second; I++) {
			if ((*I).second == ref_num) {
				index.erase(I);
				break;
			}
		}
	}
}

Result FileBase::getReference(const std::wstring& path,
							DWORDLONG& ref) const {
	WaitForSingleObject(hMutex, INFINITE);

	vector<wstring> splited_path;
	if (!this->splitPath(path, splited_path)) {
		ReleaseMutex(hMutex);
		return Result::PATH_INVALID;
	}

	vector<DWORDLONG> queue;
	for (auto& q : roots)
		queue.push_back(q);
	for (const wstring& filename : splited_path) {
		bool found = false;
		for (auto& ref_ : queue) {
			if (refmap.at(ref_).name == filename) {
				ref = ref_;
				found = true;
				queue.clear();
				this->getAllSons(ref_, queue);
				break;
			}
		}
		if (!found) {
			ReleaseMutex(hMutex);
			return Result::PATH_INVALID;
		}
	}

	ReleaseMutex(hMutex);
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

void FileBase::DFS(DWORDLONG cur, bool need_deleting, const vector<DWORDLONG>& to_delete) const {
	/*size_t s, e;
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
	}*/
}

void FileBase::BFS(DWORDLONG root, vector<DWORDLONG>& trace) const {
	queue<DWORDLONG> que;
	while (!que.empty())
		que.pop();
	trace.push_back(root);
	if (firstSon.find(root) != firstSon.end()) {
		que.push(firstSon.at(root));
	}
	while (!que.empty()) {
		auto cur = que.front();
		que.pop();
		trace.push_back(cur);
		if (firstSon.find(cur) != firstSon.end()) {
			que.push(firstSon.at(cur));
		}
		if (nextSibling.find(cur) != nextSibling.end()) {
			que.push(nextSibling.at(cur));
		}
	}
}

void FileBase::getAllSons(DWORDLONG cur, vector<DWORDLONG>& sons) const {
	if (firstSon.find(cur) == firstSon.end()) {
		return;
	}
	DWORDLONG x = firstSon.at(cur);
	sons.push_back(x);
	while (nextSibling.find(x) != nextSibling.end()) {
		x = nextSibling.at(x);
		sons.push_back(x);
	}
}

void FileBase::deleteTreeLink(DWORDLONG ref_num, DWORDLONG par_num) {
	vector<DWORDLONG> allSons;
	getAllSons(par_num, allSons);
	size_t idx = 0;
	while (idx < allSons.size()) {
		if (allSons[idx] == ref_num) {
			break;
		}
		idx++;
	}

	if (idx >= allSons.size()) {
		return;
	}
	if (allSons.size() == 1) {
		this->firstSon.erase(par_num);
		return;
	}

	if (idx == 0) {
		firstSon[par_num] = allSons[1];
	} else
	if (idx == allSons.size()-1) {
		nextSibling.erase(allSons[idx-1]);
	}
	else
	{
		nextSibling[allSons[idx - 1]] = allSons[idx + 1];
	}
	nextSibling.erase(ref_num);
}
