#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <regex>
#include <filesystem>

class TextFile
{
public:
	TextFile(const std::string& path, const std::string& name):
		_Path(path),
		_Name(name)
	{
		_Input.open(path.c_str());

		if (_Input.is_open())
		{
			std::string line;

			while (std::getline(_Input, line))
			{
				line.push_back('\n');
				_Strings.push_back(line);
			}

			_Input.close();
		}
	}

	const std::string& Path()
	{
		return _Path;
	}

	const std::string& Name()
	{
		return _Name;
	}

	size_t Length()
	{
		return _Strings.size();
	}

	std::string& String(size_t index)
	{
		return _Strings.at(index);
	}

	void replace(std::string& str, const std::string& from, const std::string& to) 
	{
		if (from.empty())
			return;
		size_t start_pos = 0;

		while ((start_pos = str.find(from, start_pos)) != std::string::npos) 
		{
			str.replace(start_pos, from.length(), to);
			start_pos += to.length();
		}
	}

	void replace(const std::string& from, const std::string& to)
	{
		for (size_t i = 0; i < _Strings.size(); i++)
		{
			replace(_Strings[i], from, to);
		}
	}
private:
	std::fstream _Input;
	std::vector<std::string> _Strings;
	std::string _Path;
	std::string _Name;
};


void FileSave(TextFile* textFile, const std::string& path)
{
	std::ofstream out(path, std::ios::trunc);

	for (size_t i = 0; i < textFile->Length(); i++)
	{
		out << textFile->String(i);
	}

	out.close();
}

void LoadFiles(std::vector<TextFile>& dest, const std::string& path, const std::string& ext)
{
	const std::regex regex("\\.\\w+");

	std::smatch smatch;

	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		std::string s = entry.path().string();

		if (std::regex_search(s, smatch, regex) && smatch[0] == ext)
		{
			std::string n = entry.path().filename().string();

			const std::regex word("\\w+");

			if (std::regex_search(n, smatch, word))
			{
			    dest.push_back(TextFile(entry.path().string(), smatch[0]));
			}
		}
	}
}

bool IsValid(const std::string& source)
{
	const std::regex regex("(typedef|static|extern|enum|pack|#define|#if|#elif)");

	std::smatch smatch;

	return std::regex_search(source, smatch, regex) != true;
}

int main()
{
	std::string path = "d:/0000/fallout2-ce/src/";

	std::vector<TextFile> headerFiles;
	std::vector<TextFile> sourceFiles;

	LoadFiles(headerFiles, path, ".h");
	LoadFiles(sourceFiles, path, ".cc");

	std::cout << headerFiles.size() << '\n';
	std::cout << headerFiles[0].Name() << '\n';
	std::cout << sourceFiles.size() << '\n';

	const std::regex r("([\\w][\\w]*\\()");

	std::smatch sm;

	std::vector<std::string> fileNames;
	std::vector<std::string> names;

	for (size_t k = 0; k < headerFiles.size(); k++)
	{
		size_t j = 0;
		size_t tr = 0;
		bool first = false;

		for (size_t i = 0; i < headerFiles[k].Length(); i++)
		{
			if (std::regex_search(headerFiles[k].String(i), sm, r) && IsValid(headerFiles[k].String(i)))
			{
				if (first == false && tr == 0)
				{
					first = true;
				}

				j = i;
				
				std::string name1 = sm[1];
				//std::string name2 = name1.substr(0, name1.size() - 1);
				std::string name2 = name1;
				std::string name3 = headerFiles[k].Name();
				//std::cout << name2 << '\n';
				names.push_back(name1);
				fileNames.push_back(name3);

				std::string s = "static " + headerFiles[k].String(i);


				if (tr == 0 && first == true)
				{
					tr = 2;

					std::string n = "class Class_" + headerFiles[k].Name() + " { public: " + s;
					s = n;
				}

				headerFiles[k].String(i) = s;

				//std::cout << s << '\n';
			}
		}


		if (j > 0)
		{
			headerFiles[k].String(j) += " };";
		}

		FileSave(&headerFiles[k], headerFiles[k].Path());

		std::cout << headerFiles[k].Path() << '\n';
	}


	for (size_t i = 0; i < sourceFiles.size(); i++)
	{
		for (size_t j = 0; j < names.size(); j++)
		{
			std::string word = names[j].substr(0, names[j].size() - 1);

			sourceFiles[i].replace(names[j], "Class_" + fileNames[j] + "::" + names[j]);

			sourceFiles[i].replace("== " + word, "== Class_" + fileNames[j] + "::" + word);

			sourceFiles[i].replace(word + ",", "Class_" + fileNames[j] + "::" + word + ",");

			sourceFiles[i].replace(word + ")", "Class_" + fileNames[j] + "::" + word + ")");

			sourceFiles[i].replace(word + ";", "Class_" + fileNames[j] + "::" + word + ";");
		}

		std::cout << sourceFiles[i].Path() << '\n';
	}


	//Post fix
	for (size_t i = 0; i < sourceFiles.size(); i++)
	{
		sourceFiles[i].replace("Class_animation::regClass_animation::_anim_animate", "Class_animation::reg_anim_animate");
		sourceFiles[i].replace("== _gsnd_anim_sound", "== Class_game_sound::_gsnd_anim_sound");
		sourceFiles[i].replace("xClass_db::", "Class_xfile::x");
		sourceFiles[i].replace("dClass_db::", "Class_dfile::d");
		sourceFiles[i].replace("worldClass_map::", "Class_map::");
		sourceFiles[i].replace("programFatalError(", "Class_interpreter::programFatalError(");	
	}

	

	for (size_t i = 0; i < sourceFiles.size(); i++)
	{
		FileSave(&sourceFiles[i], sourceFiles[i].Path());

		std::cout << sourceFiles[i].Path() << '\n';
	}

	return 0;
}