#pragma once

#ifdef _DEBUG
#undef _DEBUG
#pragma push_macro("slots")
#undef slots
#include <Python.h>
#pragma pop_macro("slots")
#define _DEBUG
#else
#pragma push_macro("slots")
#undef slots
#include <Python.h>
#pragma pop_macro("slots")
#endif

#include <QThread>
#include <qfileinfo.h>
#include <QMetaType>
#include "FileContent.h"
#include "SearchResult.h"

Q_DECLARE_METATYPE(Nothing::SearchResult);

class ContentFileSearch : public QThread
{
	Q_OBJECT

public:
	ContentFileSearch(QObject* parent, Nothing::FileContent* contentSearch, std::wstring key, std::wstring con);
	~ContentFileSearch();

protected:
	void run();

public:
	bool searching;

private:
	Nothing::FileContent *content_search;
	std::wstring keyword;
	std::wstring content;

signals:
	void foundFile(Nothing::SearchResult tmp_res);
	void finished();
};
