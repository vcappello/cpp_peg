#ifndef PEG_H
#define PEG_H

#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <limits>
#include <tuple>
#include <functional>

namespace peg
{

    /// @brief Save and restore stream status
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

    class rule_inserter_base
    {
    public:
        virtual void insert(const std::string &in_string) = 0;
    };

    template <class ContainerT>
    class rulse_inserter : public rule_inserter_base
    {
    public:
        ContainerT m_container;

        void insert(const std::string &in_string)
        {
            std::back_insert_iterator<ContainerT> ins(m_container);
            ins = in_string;
        }
    };

    /// @brief Basic abstract class for PEG rules
    class rule
    {
    public:
        using stream_t = std::istream;
        using char_t = stream_t::char_type;
        using rule_ptr_t = std::unique_ptr<rule>;

        virtual std::tuple<bool, std::string> parse(stream_t &in_istream, rule_inserter_base *in_inserter) = 0;
    };

    /// @brief Match source stream VS the defined text
    class literal : public rule
    {
    public:
        std::string m_text;

        literal(const std::string &in_text) : m_text(in_text)
        {
        }

        std::tuple<bool, std::string> parse(stream_t &is, rule_inserter_base *in_inserter) override
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

    /// @brief Match if the first character of the source stream is in the
    /// defined char range
    class range : public rule
    {
    public:
        char_t m_low;
        char_t m_high;

        range(const char_t in_low, const char_t in_high) : m_low(in_low),
                                                           m_high(in_high)
        {
        }

        std::tuple<bool, std::string> parse(stream_t &is, rule_inserter_base *in_inserter) override
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

    /// @brief Match any character in the source stream
    class any_char : public rule
    {
    public:
        std::tuple<bool, std::string> parse(stream_t &is, rule_inserter_base *in_inserter) override
        {
            char_t c;

            stream_tran tran(is);
            is.get(c);

            if (is.good())
            {
                return std::make_tuple(true, std::string(1, c));
            }

            tran.rollback(is);
            return std::make_tuple(false, "");
        }
    };

    auto any = std::make_unique<any_char>();

    /// @brief Match the second rule if the first rule doesn't match
    class neg : public rule
    {
    public:
        rule_ptr_t m_child_not;
        rule_ptr_t m_child_eq;

        neg(rule_ptr_t &&in_child_not, rule_ptr_t &&in_child_eq) : m_child_not(std::move(in_child_not)),
                                                                   m_child_eq(std::move(in_child_eq))
        {
        }

        std::tuple<bool, std::string> parse(stream_t &is, rule_inserter_base *in_inserter) override
        {
            stream_tran tran(is);
            auto [res, text] = m_child_not->parse(is, in_inserter);

            if (!res && is.good())
            {
                return m_child_eq->parse(is, in_inserter);
            }

            tran.rollback(is);
            return std::make_tuple(false, "");
        }
    };

    /// @brief Match if any of given rules match
    class choice : public rule
    {
    public:
        std::vector<rule_ptr_t> m_children;

        choice(std::vector<rule_ptr_t>&& in_children) : m_children(std::move(in_children))
        {
        }

        std::tuple<bool, std::string> parse(stream_t &is, rule_inserter_base *in_inserter) override
        {
            stream_tran tran(is);

            for (auto &child : m_children)
            {
                auto [res, text] = child->parse(is, in_inserter);
                if (res)
                {
                    return std::make_tuple(true, text);
                }
            }

            tran.rollback(is);
            return std::make_tuple(false, "");
        }
    };

    /// @brief Match if all given rules match in the given order
    class sequence : public rule
    {
    public:
        std::vector<rule_ptr_t> m_children;

        sequence(std::vector<rule_ptr_t>&& in_children) : m_children(std::move(in_children))
        {
        }

        std::tuple<bool, std::string> parse(stream_t &is, rule_inserter_base *in_inserter) override
        {
            stream_tran tran(is);
            std::stringstream text_ret;

            for (auto &child : m_children)
            {
                auto [res, text] = child->parse(is, in_inserter);
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

    /// @brief Match a rule repetion using lower and higher limits
    class repeat : public rule
    {
    public:
        rule_ptr_t m_child;
        std::size_t m_min;
        std::size_t m_max;

        static constexpr std::size_t n = std::numeric_limits<std::size_t>::max();

        repeat(rule_ptr_t &&in_child, std::size_t in_min, std::size_t in_max) : m_child(std::move(in_child)),
                                                                                m_min(in_min),
                                                                                m_max(in_max)
        {
        }

        std::tuple<bool, std::string> parse(stream_t &is, rule_inserter_base *in_inserter) override
        {
            stream_tran tran(is);
            std::size_t counter = 0;
            std::stringstream text_ret;

            while (!is.eof())
            {
                stream_tran sub_tran(is);

                auto [res, text] = m_child->parse(is, in_inserter);
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

    /// @brief Capture child rule
    class capture : public rule
    {
    public:
        using func_t = std::function<void(const std::string &)>;

        rule_ptr_t m_child;
        std::string m_id;
        func_t m_func;

        capture(rule_ptr_t &&in_child, const std::string &in_id) : m_child(std::move(in_child)),
                                                                   m_id(in_id)
        {
        }

        capture(rule_ptr_t &&in_child, const std::string &in_id, func_t in_func) : m_child(std::move(in_child)),
                                                                                   m_id(in_id),
                                                                                   m_func(in_func)
        {
        }

        std::tuple<bool, std::string> parse(stream_t &is, rule_inserter_base *in_inserter) override
        {
            auto [res, text] = m_child->parse(is, in_inserter);

            if (res)
            {
                if (m_func)
                    m_func(text);

                in_inserter->insert(text);

                return std::make_tuple(true, text);
            }

            return std::make_tuple(false, "");
        }
    };

    class ref : public rule
    {
    public:
        rule *m_child;

        ref(rule *in_child) : m_child(in_child)
        {
        }

        std::tuple<bool, std::string> parse(stream_t &is, rule_inserter_base *in_inserter) override
        {
            return m_child->parse(is, in_inserter);
        }
    };

    template <class T>
    struct movable_il
    {
        mutable T t;

        operator T() const && { return std::move(t); }

        movable_il(T &&in) : t(std::move(in)) {}

        template <typename U>
        movable_il(U &&in) : t(std::forward<U>(in)) {}
    };

    template <class T, class A = std::allocator<T>>
    std::vector<T, A> lst(std::initializer_list<movable_il<T>> il)
    {
        std::vector<T, A> r(std::make_move_iterator(il.begin()), std::make_move_iterator(il.end()));
        return r;
    }
}

#endif
