//Made in 5 minutes...could be cleaned up a bit...
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

struct question_t
{
	std::string text;
	std::string correct_answer;
	std::vector<std::string> wrong_answers;
};

//Windows CRs...
std::string strip_whitespace(std::string str)
{
	while(str.size()>0&&isspace(str[str.size()-1])!=0)
		str.pop_back();

	return str;
}

int main()
{
	std::string filename="questions.ini";
	std::ifstream istr(filename);

	if(!istr)
	{
		std::cout<<"Could not open \""<<filename<<"\"."<<std::endl;
		return 0;
	}

	std::string line;
	std::vector<question_t> questions;
	question_t question;

	while(std::getline(istr,line))
	{
		line=strip_whitespace(line);

		if(line.size()!=0)
		{
			if(line.find("?")!=std::string::npos)
			{
				if(question.correct_answer.size()!=0&&question.wrong_answers.size()!=0)
					questions.push_back(question);

				question.correct_answer="";
				question.wrong_answers.clear();
				question.text=line;
			}
			else
			{
				if(question.correct_answer.size()==0)
					question.correct_answer=line;
				else
					question.wrong_answers.push_back(line);
			}
		}
	}

	if(question.correct_answer.size()!=0&&question.wrong_answers.size()!=0)
		questions.push_back(question);

	//This just spits out all the questions...in order from the file...randomize output and call it good?
	for(size_t qq=0;qq<questions.size();++qq)
	{
		//Print Out Question
		std::cout<<qq+1<<")  "<<questions[qq].text<<std::endl;

		//Print Out Correct Answer
		std::cout<<"\ta)  "<<questions[qq].correct_answer<<std::endl;

		//Print Out Wrong Answers
		for(size_t ww=0;ww<std::min(questions[qq].wrong_answers.size(),(size_t)25);++ww)
			std::cout<<"\t"<<(char)('b'+ww)<<")  "<<questions[qq].wrong_answers[ww]<<std::endl;

		std::cout<<std::endl;
	}

	return 0;
}