#include "reader.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <stdexcept>

//Windows CRs...
static std::string strip_whitespace(std::string str)
{
	while(str.size()>0&&isspace(str[str.size()-1])!=0)
		str.pop_back();

	return str;
}

std::vector<question_t> read_questions(const std::string& filename)
{
	std::ifstream istr(filename);
	std::string line;
	std::vector<question_t> questions;
	question_t question;

	if(!istr)
		throw std::runtime_error("Could not open any file named \""+filename+"\".");

	while(std::getline(istr,line))
	{
		line=strip_whitespace(line);

		if(line.size()!=0)
		{
			if(line.find("?")==std::string::npos)
			{
				question.answers.push_back(line);
				continue;
			}

			if(question.answers.size()!=0)
				questions.push_back(question);

			question.answers.clear();
			question.text=line;
		}
	}

	if(question.answers.size()!=0)
		questions.push_back(question);

	return questions;
}

bool ask_question(const question_t& question)
{
	if(question.answers.size()>0)
	{
		std::cout<<question.text<<std::endl;

		auto random_answers=question.answers;
		std::random_shuffle(random_answers.begin(),random_answers.end());
		print_answers(random_answers);

		std::cout<<"Enter an answer:  "<<std::flush;

		std::string input;

		while(true)
		{
			if(!getline(std::cin,input))
				throw std::runtime_error("Error writing to cin.");

			if(input.size()!=1||input[0]<'a'||(size_t)(input[0]-'a')>=std::min(random_answers.size(),(size_t)26))
			{
				std::cout<<"\tInvalid input!"<<std::endl;
				std::cout<<"Try again please:  "<<std::flush;
				continue;
			}

			if(random_answers[(size_t)(input[0]-'a')]!=question.answers[0])
			{
				std::cout<<"\tIncorrect!"<<std::endl;
				return false;
			}

			std::cout<<"\tCorrect!\n"<<std::endl;
			return true;
		}
	}

	return false;
}

void print_answers(const answers_t& answers)
{
	for(size_t ii=0;ii<std::min(answers.size(),(size_t)26);++ii)
		std::cout<<"\t"<<(char)(ii+'a')<<")  "<<answers[ii]<<std::endl;
}
