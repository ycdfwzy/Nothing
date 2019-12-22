#include "ContentFileSearch.h"
#include "SearchResult.h"
#include <qdebug.h>

ContentFileSearch::ContentFileSearch(QObject *parent, Nothing::FileContent* contentSearch, std::wstring key, std::wstring con)
	: QThread(parent)
{
	content_search = contentSearch;
	keyword = key;
	content = con;
	qRegisterMetaType<Nothing::SearchResult>("SearchResult");
}

ContentFileSearch::~ContentFileSearch()
{
}

void ContentFileSearch::run() {
	searching = true;
	Nothing::SearchResult tmp_res;
	while (searching && content_search->next(keyword, content, tmp_res) != Nothing::Result::FILEPOOL_EMPTY) {
		if (tmp_res.get_content_results().size() > 0) {
			// qDebug() << "in content found file: " << tmp_res.get_name();
			emit(foundFile(tmp_res));
			tmp_res = Nothing::SearchResult();
		}
	}
	emit(finished());
}
