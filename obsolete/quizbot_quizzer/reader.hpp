#ifndef READER_C11_HPP
#define READER_C11_HPP

#include <string>
#include <vector>

typedef std::vector<std::string> answers_t;

struct question_t
{
	std::string text;
	answers_t answers;
};

std::vector<question_t> read_questions(const std::string& filename);

bool ask_question(const question_t& question);

void print_answers(const answers_t& answers);

#endif