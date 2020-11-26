#pragma once
#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/uri.hpp>
#include <filesystem> // For remove_dot_segments

    namespace isto::uri
{

    struct
uri_components_t
{
        std::string
    scheme;
        std::string
    authority;
        std::string
    path;
        std::string
    query;
        std::string
    fragment;
        friend bool
    operator == (uri_components_t const&, uri_components_t const&) = default;
};

    namespace
detail // {{{
{
    // Setup parsing.
    template <typename Rule>
    struct 
action_t
    : tao::pegtl::nothing <Rule>
{};

#define GENERATE_ACTION_2(RULE,COMPONENT)                 \
    template <>                                           \
    struct                                                \
action_t <tao::pegtl::uri::RULE>                          \
{                                                         \
        template <typename Input>                         \
        static void                                       \
    apply (const Input& in, uri_components_t& components) \
    {                                                     \
        components.COMPONENT = in.string ();              \
    }                                                     \
};
#define GENERATE_ACTION_1(RULE) GENERATE_ACTION_2(RULE,RULE)
GENERATE_ACTION_1(scheme)
GENERATE_ACTION_1(authority)
GENERATE_ACTION_2(path_abempty,path)
GENERATE_ACTION_2(path_absolute,path)
GENERATE_ACTION_2(path_rootless,path)
GENERATE_ACTION_2(path_noscheme,path)
GENERATE_ACTION_1(query)
GENERATE_ACTION_1(fragment)
#undef GENERATE_ACTION_1
#undef GENERATE_ACTION_2

    template <typename Rule>
    struct 
control_t
    : tao::pegtl::normal <Rule>
{};

// Clears the state in case of failure.
#define GENERATE_CONTROL(RULE)                           \
    template <>                                          \
    struct                                               \
control_t <tao::pegtl::uri::RULE>                        \
    : tao::pegtl::normal <tao::pegtl::uri::RULE>         \
{                                                        \
        template <typename Input>                        \
        static void                                      \
    failure (const Input&, uri_components_t& components) \
    {                                                    \
        components = {};                                 \
    }                                                    \
};
// A `URI_reference` is first tried as a `URI` and then a `relative_ref`
// if the former fails. If it does we have to clear the state.
GENERATE_CONTROL(URI)

    template <class StartingRule>
    uri_components_t
parse (std::string_view string)
{
        uri_components_t
    components;
    tao::pegtl::memory_input in (string.data (), string.length (), "");
    tao::pegtl::parse <
          StartingRule
        , detail::action_t
        , detail::control_t
    > (in, components);
    return components;
}

} // }}} namespace detail

// Free functions

// RFC 3986 § 3.
    uri_components_t
parse_uri (std::string_view string)
{
    return detail::parse <tao::pegtl::uri::URI> (string);
}
// RFC 3986 § 4.1.
    uri_components_t
parse_reference (std::string_view string)
{
    return detail::parse <tao::pegtl::uri::URI_reference> (string);
}

// RFC 3986 § 4.3.
    uri_components_t
parse_absolute_uri (std::string_view string)
{
    return detail::parse <tao::pegtl::uri::absolute_URI> (string);
}

// RFC 3986 § 5.2.2.
// TODO: maybe the name isn't right.
    uri_components_t
resolve (uri_components_t const& base, std::string_view relative)
{
        uri_components_t
    R, T;
        uri_components_t const&
    Base = base;

        auto
    merge = [&]() -> std::string
    {
        if (!Base.authority.empty () && Base.path.empty ())
        {
            return "/" + R.path;
        }
            auto
        pos = Base.path.rfind ('/');
        if (pos == std::string::npos)
        {
            return R.path;
        }
        else
        {
            return Base.path.substr (0, pos) + '/' + R.path;
        }
    };
        auto
    remove_dot_segments = [](std::string_view path)
    {
            std::filesystem::path
        p (path);
        return p.lexically_normal ().string ();
    };

    R = parse_reference (relative);
    if (!R.scheme.empty ())
    {
        T.scheme    = R.scheme;
        T.authority = R.authority;
        T.path      = remove_dot_segments (R.path);
        T.query     = R.query;
    }
    else
    {
        if (!R.authority.empty ())
        {
            T.authority = R.authority;
            T.path      = remove_dot_segments (R.path);
            T.query     = R.query;
        }
        else
        {
            if (R.path.empty ()) 
            {
                T.path = Base.path;
                if (!R.query.empty ())
                {
                    T.query = R.query;
                }
                else
                {
                    T.query = Base.query;
                }
            }
            else
            {
                if (R.path[0] == '/')
                {
                    T.path = remove_dot_segments (R.path);
                }
                else
                {
                    T.path = merge (/*Base.path, R.path*/);
                    T.path = remove_dot_segments (T.path);
                }
                T.query = R.query;
            }
            T.authority = Base.authority;
        }
        T.scheme = Base.scheme;
    }
    T.fragment = R.fragment;
    return T;
}

// percent-escaped characters -> characters
    std::string
decode_percent (std::string_view input)
{
        std::string
    output;
    output.reserve (input.size ());
    for (auto&& i = input.begin (); i != input.end (); ++i)
    {
        if (*i != '%') 
        {
            output.push_back (*i);
            continue;
        }
        if (++i == input.end ()) 
        {
            output.push_back ('%');
            break;
        }
            char
        a = *i;
        if (++i == input.end ()) 
        {
            output.push_back ('%');
            output.push_back (a);
            break;
        }
            char
        b = *i;
            char
        r = 
              (a <= '9' ? a - '0': 10 + (a <= 'F' ? a - 'A' : a - 'f')) * 16
            + (b <= '9' ? b - '0': 10 + (b <= 'F' ? b - 'A' : b - 'f'))
        ;
        output.push_back (r);
    }
    return output;
}

// RFC 3986 § 5.3.
    std::string
recompose (uri_components_t const& components)
{
        std::string
    result;

    if (!components.scheme.empty ())
    {
        result += components.scheme + ':';
    }

    if (!components.authority.empty ())
    {
        result += "//" + components.authority;
    }
    result += components.path;
    if (!components.query.empty ())
    {
        result += '?' + components.query;
    }

    if (!components.fragment.empty ())
    {
        result += '#' + components.fragment;
    }
    return result;
}

// All bundled-up in a class
    class
uri_t
{
        uri_components_t
    components_m;

    uri_t (uri_components_t&& components)
        : components_m { std::move (components) }
    {}

public:
    uri_t ()                   = default;

    uri_t (uri_t const&)       = default;

    uri_t (uri_t&&)            = default;

        uri_t&
    operator = (uri_t const&)  = default;

        friend void
    swap (uri_t& a, uri_t& b)
    {
        std::swap (a.components_m, b.components_m);
    }

    // Very naive. See RFC 3986 § 6.
        friend bool
    operator == (uri_t const& a, uri_t const& b)
    {
        return a.components_m == b.components_m;
    }

        friend bool
    operator != (uri_t const& a, uri_t const& b)
    {
        return a.components_m != b.components_m;
    }
    // At this point, the type is Regular.


    // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0994r0.html
    uri_t (std::string_view string)
        : components_m { parse_uri (string) }
    {}

    uri_t (std::string const& string)
        : components_m { parse_uri (string) }
    {}

    uri_t (const char* string)
        : components_m { parse_uri (string) }
    {}

        uri_t&
    operator = (std::string_view string)
    {
        components_m = parse_uri (string);
        return *this;
    }

    operator std::string () const
    {
        return string ();
    }

        std::string
    string () const
    {
        return recompose (components_m);
    }

        uri_t
    resolve (std::string_view relative_reference) const
    {
        return isto::uri::resolve (components_m, relative_reference);
    }

        std::string
    decode_percent () const
    {
        return isto::uri::decode_percent (string ());
    }

        std::string_view
    fragment () const
    {
        return components_m.fragment;
    }

        void
    clear_fragment ()
    {
        components_m.fragment = {};
    }

        uri_t
    absolute () const
    {
        if (components_m.fragment.empty ()) return *this;
            uri_components_t
        c = components_m;
        c.fragment.clear ();
        return c;
    }
};

} // namespace isto::uri
