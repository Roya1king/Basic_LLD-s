// Designing Stack Overflow
// Requirements
// Users can post questions, answer questions, and comment on questions and answers.
// Users can vote on questions and answers.
// Questions should have tags associated with them.
// Users can search for questions based on keywords, tags, or user profiles.
// The system should assign reputation score to users based on their activity and the quality of their contributions.

#include <bits/stdc++.h>
#include <random>
#include <unordered_set>
using namespace std;

const int REPUTATION_FOR_QUESTION = 5;
const int REPUTATION_FOR_ANSWER = 10;
const int REPUTATION_FOR_QUESTION_UPVOTE = 10;
const int REPUTATION_FOR_ANSWER_UPVOTE = 15;
const int REPUTATION_FOR_DOWNVOTE = 2;
// const int REPUTATION_FOR_VOTER_DOWNVOTE = 1;

enum class voteType
{
    Upvote,
    Downvote
};

string generateRandomString(int length = 10)
{
    static const char charset[] =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789";
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_int_distribution<> dis(0, sizeof(charset) - 2);

    string result;
    result.reserve(length);
    for (int i = 0; i < length; ++i)
        result += charset[dis(gen)];
    return result;
}

string toLower(const string &str)
{
    string result = str;
    transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

class User
{
private:
    string username;
    string Name;
    int reputation = 0;

public:
    User(string uname, string name) : username(uname), Name(name) {};
    string getUsername() const { return username; }
    string getName() const { return Name; }

    int getReputation()
    {
        return reputation;
    }

    void addReputation(int point)
    {
        reputation += point;
    }

    void reduceReputation(int point)
    {
        reputation -= point;
    }
};

class Tags
{
private:
    string tagName;

public:
    Tags(string tagName) : tagName(tagName) {};

    string getTag()
    {
        return tagName;
    }
};

class Vote
{
private:
    shared_ptr<User> user;
    voteType type;

public:
    Vote(shared_ptr<User> user, voteType vote) : user(user), type(vote) {};
    ~Vote() = default;

    voteType getVoteType()
    {
        return type;
    }

    shared_ptr<User> getuser()
    {
        return user;
    }

    void alterVote()
    {
        if (getVoteType() == voteType::Downvote)
        {
            type = voteType::Upvote;
        }
        else
        {
            type = voteType::Downvote;
        }
    }
};

class Comments
{
protected:
    string commentId;
    string commentText;
    shared_ptr<User> user;
    // commentType type;

public:
    Comments(string cId, string comment, shared_ptr<User> userPtr)
        : commentId(cId), commentText(comment), user(userPtr) {}
};

class Answer
{
private:
    string answerId;
    string answerText;
    shared_ptr<User> user;
    vector<shared_ptr<Vote>> votes;
    vector<shared_ptr<Comments>> comments;
    int upVote = 0;
    int downVote = 0;

public:
    Answer(string answerId, string answerText, shared_ptr<User> user) : answerId(answerId), answerText(answerText), user(user) {};

    string getAnswerText()
    {
        return answerText;
    }

    shared_ptr<User> getUser()
    {
        return user;
    }

    void addVote(shared_ptr<User> U, voteType V)
    {
        for (auto it = votes.begin(); it != votes.end(); ++it)
        {
            if ((*it)->getuser() == U)
            {
                if ((*it)->getVoteType() == V)
                {
                    if (V == voteType::Upvote)
                    {
                        upVote--;
                        user->addReputation(-REPUTATION_FOR_ANSWER_UPVOTE);
                    }
                    else
                    {
                        downVote--;
                        user->reduceReputation(REPUTATION_FOR_DOWNVOTE);
                    }
                    votes.erase(it); // Erase the vote object
                    return;
                }
                else
                {
                    (*it)->alterVote();
                    if (V == voteType::Upvote)
                    {
                        upVote++;
                        user->addReputation(REPUTATION_FOR_ANSWER_UPVOTE);
                        downVote--;
                        user->reduceReputation(REPUTATION_FOR_DOWNVOTE);
                    }
                    else
                    {
                        upVote--;
                        user->addReputation(-REPUTATION_FOR_ANSWER_UPVOTE);
                        downVote++;
                        user->reduceReputation(-REPUTATION_FOR_DOWNVOTE);
                    }
                    return;
                }
            }
        }
        // If user hasn't voted yet, add new vote
        auto newVote = make_shared<Vote>(U, V);
        votes.push_back(newVote);
        if (V == voteType::Upvote)
        {
            upVote++;
            user->addReputation(REPUTATION_FOR_ANSWER_UPVOTE);
        }
        else
        {
            downVote++;
            user->reduceReputation(REPUTATION_FOR_DOWNVOTE);
        }
    }

    int getUpvote()
    {
        return upVote;
    }

    int getDownVote()
    {
        return downVote;
    }

    void addComment(shared_ptr<User> U, const string &commentText)
    {
        string cId = generateRandomString();
        auto newComment = make_shared<Comments>(cId, commentText, U);
        comments.push_back(newComment);
    }

    vector<shared_ptr<Comments>> getComments(){
        return comments;
    }
};

class Question
{
private:
    string questionId;
    string questionText;
    vector<shared_ptr<Answer>> answers;
    vector<shared_ptr<Tags>> tags;
    shared_ptr<User> user;
    vector<shared_ptr<Vote>> votes;
    vector<shared_ptr<Comments>> comments;
    int upVote = 0;
    int downVote = 0;

public:
    Question(string qId, string qText, shared_ptr<User> user) : questionId(qId), questionText(qText), user(user) {};
    void addTag(shared_ptr<Tags> tag)
    {
        tags.push_back(tag);
    }

    string getUsername()
    {
        return user->getName();
    }

    string getQuestionText()
    {
        return questionText;
    }

    unordered_set<string> getTags()
    {
        unordered_set<string> temp;
        for (auto &T : tags)
        {
            temp.insert(T->getTag());
        }
        return temp;
    }

    shared_ptr<Answer> addAnswer(shared_ptr<User> U, const string &anstext)
    {
        string ansId = generateRandomString();
        auto A = make_shared<Answer>(ansId, anstext, U);
        answers.push_back(A);
        return A;
    }

    vector<shared_ptr<Answer>> getAnswers()
    {
        return answers;
    }

    void addVote(shared_ptr<User> U, voteType V)
    {
        for (auto it = votes.begin(); it != votes.end(); ++it)
        {
            if ((*it)->getuser() == U)
            {
                if ((*it)->getVoteType() == V)
                {
                    if (V == voteType::Upvote)
                    {
                        upVote--;
                        user->addReputation(-REPUTATION_FOR_QUESTION_UPVOTE);
                    }
                    else
                    {
                        downVote--;
                        user->reduceReputation(REPUTATION_FOR_DOWNVOTE);
                    }
                    votes.erase(it); // Erase the vote object
                    return;
                }
                else
                {
                    (*it)->alterVote();
                    if (V == voteType::Upvote)
                    {
                        upVote++;
                        user->addReputation(REPUTATION_FOR_QUESTION_UPVOTE);
                        downVote--;
                        user->reduceReputation(REPUTATION_FOR_DOWNVOTE);
                    }
                    else
                    {
                        upVote--;
                        user->addReputation(-REPUTATION_FOR_QUESTION_UPVOTE);
                        downVote++;
                        user->reduceReputation(-REPUTATION_FOR_DOWNVOTE);
                    }
                    return;
                }
            }
        }
        // If user hasn't voted yet, add new vote
        auto newVote = make_shared<Vote>(U, V);
        votes.push_back(newVote);
        if (V == voteType::Upvote)
        {
            upVote++;
            user->addReputation(REPUTATION_FOR_QUESTION_UPVOTE);
        }
        else
        {
            downVote++;
            user->reduceReputation(REPUTATION_FOR_DOWNVOTE);
        }
    }

    int getUpvote()
    {
        return upVote;
    }

    int getDownVote()
    {
        return downVote;
    }

    void addComment(shared_ptr<User> U, const string &commentText)
    {
        string cId = generateRandomString();
        auto newComment = make_shared<Comments>(cId, commentText, U);
        comments.push_back(newComment);
    }

    void addCommentOnAnswer(shared_ptr<User> U, shared_ptr<Answer> A, const string &commentText)
    {
        if (find(answers.begin(), answers.end(), A) == answers.end())
        {
            return;
        }
        A->addComment(U, commentText);
    }

    vector<shared_ptr<Comments>> getComments(){
        return comments;
    }
};

class StackOverflow
{
private:
    vector<shared_ptr<Question>> questions;
    vector<shared_ptr<User>> user;
    unordered_set<string> userCheck;

public:
    // creating user
    shared_ptr<User> createUser(string uname, string name)
    {
        if (userCheck.find(uname) == userCheck.end())
        {
            auto U = make_shared<User>(uname, name);
            user.push_back(U);
            userCheck.insert(uname);
            return U;
        }
        else
        {
            cout << "Username already exits, please change username" << endl;
            return nullptr;
        }
    }

    shared_ptr<Question> addQuestion(shared_ptr<User> U, const string &text)
    {
        string rndId = generateRandomString();
        auto Q = make_shared<Question>(rndId, text, U);
        U->addReputation(REPUTATION_FOR_QUESTION);
        questions.push_back(Q);
        return Q;
    }

    unordered_set<shared_ptr<Question>> findQuestion(string key)
    {
        unordered_set<shared_ptr<Question>> response;
        string lowerKey = toLower(key);

        for (auto &Q : questions)
        {
            if (toLower(Q->getQuestionText()).find(lowerKey) != string::npos)
            {
                response.insert(Q);
                continue;
            }
            if (toLower(Q->getUsername()).find(lowerKey) != string::npos)
            {
                response.insert(Q);
                continue;
            }
            auto tags = Q->getTags();
            for (const auto &tag : tags)
            {
                if (toLower(tag).find(lowerKey) != string::npos)
                {
                    response.insert(Q);
                    break;
                }
            }
        }
        return response;
    }

    shared_ptr<Answer> answerQuestion(shared_ptr<User> U, shared_ptr<Question> Q, const string &answerText)
    {
        shared_ptr<Answer> ans = Q->addAnswer(U, answerText);
        U->addReputation(REPUTATION_FOR_ANSWER);
        return ans;
    };

    void addCommentOnQuestion(shared_ptr<User> U, shared_ptr<Question> Q, string commentText)
    {
        string trimmed = commentText;
        trimmed.erase(remove_if(trimmed.begin(), trimmed.end(), ::isspace), trimmed.end());

        if (trimmed.empty())
        {
            cout << "Comment cannot be empty or only spaces." << endl;
            return;
        }
        Q->addComment(U, commentText);
    }

    void addCommentOnAnswer(shared_ptr<User> U, shared_ptr<Question> Q, shared_ptr<Answer> A, const string &commenttext)
    {
        string trimmed = commenttext;
        trimmed.erase(remove_if(trimmed.begin(), trimmed.end(), ::isspace), trimmed.end());

        if (trimmed.empty())
        {
            cout << "Comment cannot be empty or only spaces." << endl;
            return;
        }
        Q->addCommentOnAnswer(U, A, commenttext);
    }
};