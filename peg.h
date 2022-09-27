#include <vector>
#include <string>
#include <sstream>
#include <limits>
#include <tuple>
#include <functional>

namespace peg
{

    class stream_tran
    {
    public:
        std::istream::pos_type m_pos;

        stream_tran(std::istream &is)
        {
            m_pos = is.tellg();
        }

        void rollback(std::istream &is)
        {
            is.seekg(m_pos);
        }
    };

    class rule
    {
    public:
        using stream_t = std::istream;
        using char_t = stream_t::char_type;
        using rule_ptr_t = rule *;

        virtual std::tuple<bool, std::string> parse(stream_t &is) = 0;
    };

    class literal : public rule
    {
    public:
        std::string m_text;

        literal(const std::string &in_text) : m_text(in_text)
        {
        }

        std::tuple<bool, std::string> parse(stream_t &is) override
        {
            char_t buffer[m_text.length() + 1];

            stream_tran tran(is);
            is.get(buffer, m_text.length() + 1);

            if (is.good())
            {
                if (m_text == buffer)
                {
                    return std::make_tuple(true, m_text);
                }
            }

            tran.rollback(is);
            return std::make_tuple(false, "");
        }
    };

    class range : public rule
    {
    public:
        char_t m_low;
        char_t m_high;

        range(char_t in_low, char_t in_high) : m_low(in_low),
                                               m_high(in_high)
        {
        }

        std::tuple<bool, std::string> parse(stream_t &is) override
        {
            char_t c;

            stream_tran tran(is);
            is.get(c);

            if (is.good())
            {
                if (c >= m_low && c <= m_high)
                {
                    return std::make_tuple(true, std::string(1, c));
                }
            }

            tran.rollback(is);
            return std::make_tuple(false, "");
        }
    };

    class neg : public rule
    {
    public:
        rule_ptr_t m_child;

        neg(rule_ptr_t in_child) : m_child(in_child)
        {
        }

        std::tuple<bool, std::string> parse(stream_t &is) override
        {
            stream_tran tran(is);
            auto [res, text] = m_child->parse(is);

            if (!res)
            {
                return std::make_tuple(true, text);
            }

            tran.rollback(is);
            return std::make_tuple(false, "");
        }
    };

    class choice : public rule
    {
    public:
        std::vector<rule_ptr_t> m_children;

        choice(std::initializer_list<rule_ptr_t> in_children) : m_children(in_children)
        {
        }

        std::tuple<bool, std::string> parse(stream_t &is) override
        {
            stream_tran tran(is);

            for (auto child : m_children)
            {
                auto [res, text] = child->parse(is);
                if (res)
                {
                    return std::make_tuple(true, text);
                }
            }

            tran.rollback(is);
            return std::make_tuple(false, "");
        }
    };

    class sequence : public rule
    {
    public:
        std::vector<rule_ptr_t> m_children;

        sequence(std::initializer_list<rule_ptr_t> in_children) : m_children(in_children)
        {
        }

        std::tuple<bool, std::string> parse(stream_t &is) override
        {
            stream_tran tran(is);
            std::stringstream text_ret;

            for (auto child : m_children)
            {
                auto [res, text] = child->parse(is);
                if (!res)
                {
                    tran.rollback(is);
                    return std::make_tuple(false, "");
                }

                text_ret << text;
            }

            return std::make_tuple(true, text_ret.str());
        }
    };

    class repeat : public rule
    {
    public:
        rule_ptr_t m_child;
        std::size_t m_min;
        std::size_t m_max;

        static const std::size_t n = std::numeric_limits<std::size_t>::max();

        repeat(rule_ptr_t in_child, std::size_t in_min, std::size_t in_max) : m_child(in_child),
                                                                              m_min(in_min),
                                                                              m_max(in_max)
        {
        }

        std::tuple<bool, std::string> parse(stream_t &is) override
        {
            stream_tran tran(is);
            std::size_t counter = 0;
            std::stringstream text_ret;

            while (!is.eof())
            {
                stream_tran sub_tran(is);

                auto [res, text] = m_child->parse(is);
                if (!res)
                {
                    sub_tran.rollback(is);

                    if (counter >= m_min && counter <= m_max)
                    {
                        return std::make_tuple(true, text_ret.str());
                    }
                    else
                    {
                        tran.rollback(is);
                        return std::make_tuple(false, "");
                    }
                }

                counter++;
                text_ret << text;

                if (counter == m_max)
                {
                    return std::make_tuple(true, text_ret.str());
                }
            }

            tran.rollback(is);
            return std::make_tuple(false, "");
        }
    };

    class capture : public rule
    {
    public:
        using func_t = std::function<void(const std::string &)>;

        rule_ptr_t m_child;
        std::string m_id;
        func_t m_func;

        capture(rule_ptr_t in_child, const std::string &in_id) : m_child(in_child),
                                                                 m_id(in_id)
        {
        }

        capture(rule_ptr_t in_child, const std::string &in_id, func_t in_func) : m_child(in_child),
                                                                                 m_id(in_id),
                                                                                 m_func(in_func)
        {
        }

        std::tuple<bool, std::string> parse(stream_t &is) override
        {
            auto [res, text] = m_child->parse(is);

            if (!res)
            {
                if (m_func)
                    m_func(text);

                return std::make_tuple(true, text);
            }

            return std::make_tuple(false, "");
        }
    };
}
