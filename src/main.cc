#include <QtCore>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <boost/bind.hpp>

using namespace std;

class FindAndReplace {
	vector<QString> m_lines;
	QHash<QString, QString> m_keys_vals;
	vector<QFuture<void> > m_threads;
	size_t m_num_threads;

	/**
	@brief Do the actual find and replace.
	*/
	void findAndReplaceThread (size_t i_begin, size_t i_end) {
		QHash<QString, QString>::const_iterator itr;
		itr = m_keys_vals.constBegin ();
		QString key, value;
		size_t count = 0;
		while (itr != m_keys_vals.constEnd ()) {
			QRegExp *reg_ex = 0;
			key = itr.key ();
			value = itr.value ();
			//cerr << ++count << endl;
			for (size_t i = i_begin; i < i_end; ++i) {
				if (m_lines[i].contains (key)) {
					if (!reg_ex) {
						reg_ex = new QRegExp ("\\b" + key +"\\b");
					}
					m_lines[i].replace (*reg_ex, value);
				}
			}
			delete reg_ex;
			reg_ex = 0;
			++itr;
		}
	}

	/**
	@brief Partition the text file into worker thread partitions.
	*/
	void setupThreads () {
		size_t n = m_lines.size () / m_num_threads;
		size_t n_begin = 0, n_end = n;
		//cout << "Total = " << m_lines.size () << endl;
		for (size_t i = 0; i < m_num_threads - 1; ++i) {
			//cout << '[' << n_begin << " : " << n_end << ')' << endl;
			m_threads.push_back (QtConcurrent::run(boost::bind (&FindAndReplace::findAndReplaceThread, this, n_begin, n_end)));
			n_begin += n;
			n_end += n;
		}
		//cout << '[' << n_begin << " : " << m_lines.size () << ')' << endl;
		m_threads.push_back (QtConcurrent::run(boost::bind (&FindAndReplace::findAndReplaceThread, this, n_begin, m_lines.size ())));
	}

	void waitForFinished () {
		for (size_t i = 0; i < m_threads.size (); ++i) {
			m_threads[i].waitForFinished ();
		}
	}

	void printToStdout () const {
		for (size_t i = 0; i < m_lines.size (); ++i) {
			cout << m_lines[i].toStdString () << endl;
		}
	}

public:

	/**
	@brief Initialize find and replace
	*/
	FindAndReplace (const char *file_name, size_t n_threads) : m_num_threads (n_threads) {
		FILE *f = fopen (file_name, "r");
		QTextStream f_text (f, QIODevice::ReadOnly);
		while (!f_text.atEnd ()) {
			m_lines.push_back (f_text.readLine ());
		}
	}

	/**
	@brief Add another dictionary for the find and replace to use.
	*/
	void addDictionary (const char *file_name) {
		QString f_line;
		FILE *f = fopen (file_name, "r");
		QTextStream *f_lower_case;

		f_lower_case = new QTextStream (f, QIODevice::ReadOnly);

		QStringList split_keys_vals;

		while (!f_lower_case->atEnd ()) {
			f_line = f_lower_case->readLine ();
			split_keys_vals = f_line.split (":", QString::SkipEmptyParts);
			m_keys_vals[split_keys_vals[0]] = split_keys_vals[1];

			split_keys_vals[0][0] = split_keys_vals[0][0].toUpper ();
			split_keys_vals[1][0] = split_keys_vals[1][0].toUpper ();
			m_keys_vals[split_keys_vals[0]] = split_keys_vals[1];
		}

		delete f_lower_case;
		f_lower_case = 0;
	}

	/**
	@brief Start the find and replace process.
	*/
	void start () {
		if (m_num_threads > 1) { //threading enabled
			setupThreads ();
			waitForFinished ();
		}
		else { //threading disabled
			findAndReplaceThread (0, m_lines.size ());
		}
		printToStdout ();
	}
};

int main (int argc, char ** argv) {
	if (argc < 3) {
		cout << "Conv_FindAndReplace [input file] [dict1 dict2 dict3]" << endl;
		exit (-1);
	}

	int n_threads = QThread::idealThreadCount ();
	n_threads = (n_threads == -1) ? 1 : n_threads;
	FindAndReplace find_rep (argv[1], n_threads);

	for (int i = 2; i < argc; ++i) {
		find_rep.addDictionary (argv[i]);
	}

	find_rep.start ();
}

