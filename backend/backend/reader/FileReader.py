import textract


class FileReader:
    def __init__(self, filepath: str):
        self.filepath = filepath

    def getContent(self):
        content = ''
        try:
            if self.filepath.endswith('.txt') or \
                    self.filepath.endswith('.cpp') or \
                    self.filepath.endswith('.c') or \
                    self.filepath.endswith('.cc') or \
                    self.filepath.endswith('.h') or \
                    self.filepath.endswith('.hpp') or \
                    self.filepath.endswith('.java') or \
                    self.filepath.endswith('.py'):
                with open(self.filepath, 'r', encoding='utf-8') as fin:
                    content = fin.read()
            elif self.filepath.endswith('.docx') or \
                    self.filepath.endswith('.xlsx') or \
                    self.filepath.endswith('.xls') or \
                    self.filepath.endswith('.pptx') or \
                    self.filepath.endswith('.pdf') or \
                    self.filepath.endswith('.html'):
                content = str(textract.process(filename=self.filepath), encoding='utf-8')
        except Exception:
            print('未读到文件：' + self.filepath)
        return content


if __name__ == '__main__':
    reader = FileReader(r'D:\Program Files\MATLAB\R2018b\help\simulink\ug\share-project-on-github.html')
    print(reader.getContent())
