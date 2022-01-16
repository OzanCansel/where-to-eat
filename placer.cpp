#include "placer.h"
#include <filesystem>
#include <fstream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <cmath>
#include <type_traits>
#include <random>

namespace placer
{

static std::random_device rd;
static std::mt19937 gen { rd() };

struct place
{
    int         id;
    int         score;
    std::string desc;

    friend bool operator ==( const place& x , const place& y )
    {
        return x.id == y.id;
    }

    friend bool operator <( const place& x , const place& y )
    {
        return x.score < y.score;
    }

    friend bool operator >( const place& x , const place& y )
    {
        return y < x;
    }
};

struct extra_score
{
    place where;
    int   extra;
};

using parse_result = std::vector<std::pair<place , int>>;

std::vector<std::string> read_lines( std::filesystem::path path , int skip = 1 )
{
    std::ifstream lines_f { path };

    if ( lines_f.fail() )
        throw std::runtime_error {
            "Error occurred while reading " +
            std::string { path }
        };

    std::vector<std::string> lines;

    std::string line;

    while( getline( lines_f , line ) )
        lines.push_back( move( line ) );

    if ( skip == 1 )
    {
        lines.erase(
            remove(
                begin( lines ) ,
                end( lines ) ,
                lines.front()
            )
        );
    }
    else if ( skip > 1 )
    {
        throw std::logic_error {
            "Not implemented functionality."
        };
    }

    return lines;
}

std::vector<place> read_places( std::filesystem::path path )
{
    auto lines = read_lines( path );

    std::vector<place> places;
    places.reserve( size( lines ) );

    transform(
        begin( lines ) ,
        end( lines ) ,
        back_inserter( places ) ,
        []( std::string line ) {
            std::stringstream ss { line };
            place p;

            ss >> p.id >> p.score >> p.desc;

            return p;
        }
    );

    return places;
}

template<typename T , typename U>
T find_or_throw( const T& searchee , U& collection )
{
    auto found = find(
        begin( collection ) ,
        end( collection ) ,
        searchee
    );

    if ( found == end( collection ) )
        throw std::runtime_error { "'searchee' could not be found." };

    return *found;
}

std::pair<place , int>&  find_or_throw(
    const place& searchee ,
    std::vector<std::pair<place , int>>& accumulator
)
{
    auto founded = find_if(
        begin( accumulator ) ,
        end( accumulator ) ,
        [ &searchee ]( const std::pair<place , int>& ap )
        {
            return ap.first == searchee;
        }
    );

    if ( founded == end( accumulator ) )
        throw std::invalid_argument {
            "'searchee' could not be found."
        };

    return *founded;
}

place read_placement(
    std::filesystem::path path ,
    const std::vector<place>& places
)
{
    if ( path.extension() != ".placement" )
        throw std::invalid_argument { "File extension is not '.placement'." };

    std::stringstream ss { read_lines( path ).front() };

    place pl;

    ss >> pl.id;

    pl = find_or_throw( pl , places );

    return pl;
}

std::vector<std::string> list_files( std::filesystem::path dir , std::string extension )
{
    if ( !is_directory( dir ) )
        throw std::invalid_argument { "Path is not a directory." };

    std::vector<std::string> placement_files;

    for ( const std::filesystem::directory_entry& e : std::filesystem::directory_iterator { dir } )
    {
        if ( e.is_directory() || e.is_symlink() )
            continue;

        auto f_p = e.path();

        if ( f_p.extension() != extension )
            continue;

        placement_files.push_back(
            canonical(
                absolute( f_p )
            )
        );
    }

    return placement_files;
}

std::vector<place> read_placements(
    std::filesystem::path directory ,
    const std::vector<place>& points
)
{
    auto placement_files = list_files( directory , ".placement" );

    std::vector<place> placements;

    for ( const std::string& placement_f : placement_files )
    {
        auto placement = read_placement( placement_f , points );

        placements.push_back( placement );
    }

    return placements;
}

std::vector<extra_score> read_extra_score(
    std::filesystem::path score_f_path ,
    const std::vector<place>& places
)
{
    std::vector<extra_score> scores;

    for ( const std::string& line : read_lines( score_f_path ) )
    {
        std::stringstream ss { line };

        extra_score score;

        ss >> score.where.id;
        ss >> score.extra;

        score.where = find_or_throw( score.where , places );

        scores.push_back( score );
    }

    return scores;
}

std::vector<extra_score> read_extra_scores(
    std::filesystem::path directory ,
    const std::vector<place>& places
)
{
    if ( !is_directory( directory ) )
        throw std::invalid_argument {
            directory.string() + " is not a directory."
        };

    std::vector<extra_score> extras;

    for ( const std::string& f_path : list_files( directory , ".extra" ) )
    {
        auto scores_for_f = read_extra_score( f_path , places );

        extras.insert(
            end( extras ) ,
            begin( scores_for_f ) ,
            end( scores_for_f )
        );
    }

    return extras;
}

void apply_extra_scores(
    const std::vector<extra_score>& extras ,
    std::vector<std::pair<place , int>>& accumulator
)
{
    for ( const extra_score& extra : extras )
    {
        auto& [ pl , r ] = find_or_throw( extra.where , accumulator );

        r += extra.extra;
    }
}

void accumulate_scores(
    const std::vector<place>& placements ,
    std::vector<std::pair<place , int>>& accumulator
)
{
    for ( const place& a : placements )
    {
        auto ac = find_if(
            begin( accumulator ) ,
            end( accumulator ) ,
            [ &a ]( const std::pair<place , int>& p ){
                return a == p.first;
            }
        );

        if ( ac == end( accumulator ) )
            throw std::runtime_error { "'place' doesn't exist." };

        ac->second += a.score;
    }
}

int find_last_idx( std::filesystem::path directory )
{
    auto placements = list_files( directory , ".placement" );
    int last_idx = 0;

    if ( !empty( placements ) )
    {
        std::vector<int> placement_ids;
        placement_ids.reserve( size( placements ) );

        transform(
            begin( placements ) ,
            end( placements ) ,
            back_inserter( placement_ids ) ,
            []( const std::string& file_path ) {
                return std::atoi(
                    std::filesystem::path { file_path }.filename().c_str()
                );
            }
        );

        last_idx = *max_element(
            begin( placement_ids ) ,
            end( placement_ids )
        );
    }

    return last_idx;
}

std::filesystem::path generate_filename_from_idx( int idx )
{
    std::stringstream ss;

    ss << std::setfill( '0' ) << std::setw( 6 ) << idx << ".placement";

    return ss.str();
}

void add_salt( std::vector<std::pair<place , int>>& scores )
{
    if ( empty( scores ))
        return;

    shuffle(
        begin( scores ) ,
        end( scores ) ,
        gen
    );

    auto max_score = max_element(
        begin( scores ) ,
        end( scores ) ,
        []( const std::pair<place , int>& lhs , const std::pair<place , int>& rhs ){
            return lhs.first.score < rhs.first.score;
        }
    )->first.score;

    std::uniform_int_distribution<int> dist( 0 , max_score );

    for_each (
        begin( scores ) ,
        end( scores ) ,
        [ &dist ]( std::pair<place , int>& score_p ){
            score_p.second += dist( gen );
        }
    );
}

void write_placement( std::filesystem::path file , const place& placement )
{
    std::ofstream placement_f { file };

    if ( placement_f.bad() )
        throw std::runtime_error {
            "Could not open file" + file.string()
        };

    placement_f << "where\n";
    placement_f << std::left
                << std::setw( 7 ) << placement.id
                << "\n";
}

void print_summary( const std::vector<std::pair<place , int>>& accumulator )
{
    auto longest_desc = max_element(
        begin( accumulator ) ,
        end( accumulator ) ,
        []( const std::pair<place , int>& lhs , const std::pair<place , int>& rhs ){
            return size( lhs.first.desc ) < size( rhs.first.desc );
        }
    );

    if ( longest_desc == end( accumulator ) )
        return;

    auto desc_len = size( longest_desc->first.desc ) + 2;

    std::cout << std::setw( 6 )        << "place"
                << std::setw( 6 )        << "total"
                << std::setw( desc_len ) << "desc"
                << "\n";

    for ( const auto& [ p , score ] : accumulator )
    {
        std::cout << std::setw( 6 )        << p.id
                    << std::setw( 6 )        << score
                    << std::setw( desc_len ) << p.desc
                    << "\n";
    }

    std::cout.flush();
}

parse_result parse( std::filesystem::path directory )
{
    if ( !is_directory( directory ) )
        throw std::invalid_argument {
            directory.string() + " is not a directory."
        };

    parse_result result;

    auto places = read_places( directory / "places" );

    transform(
        begin( places ) ,
        end( places ) ,
        back_inserter( result ) ,
        []( const place& ap ) {
            return std::make_pair( ap , 0 );
        }
    );

    auto extras = read_extra_scores( directory , places );

    apply_extra_scores( extras , result );

    auto placements_history = read_placements(
        directory ,
        places
    );

    accumulate_scores( placements_history , result );

    return result;
}

void next( std::filesystem::path root_p )
{
    auto result = parse( root_p );

    struct ascending_score
    {
        using entry = std::pair<place , int>;

        bool operator()( const entry& x , const entry& y )
        {
            return x.second < y.second;
        }
    };

    add_salt( result );

    sort(
        begin( result ) ,
        end( result ) ,
        ascending_score {}
    );

    auto same_points_bound = partition(
        begin( result ) ,
        end( result ) ,
        [ &result ]( std::pair<place , int> other ){
            return result.front().second == other.second;
        }
    );

    shuffle(
        begin( result ) ,
        same_points_bound ,
        gen
    );

    place new_placement;

    new_placement = result.front().first;

    auto next_idx = find_last_idx( root_p ) + 1;
    auto filename = generate_filename_from_idx( next_idx );

    write_placement( root_p / filename , new_placement );
    print_placement( root_p / filename );
}

void summary( std::filesystem::path root )
{
    auto result = parse( root );

    print_summary( result );
}

template<typename T>
int num_of_digits( T num )
{
    static_assert( std::is_integral_v<T> , "T must be a integral type." );

    return std::floor( std::log10( num ) + 1 );
}

void print_placement( std::filesystem::path placement_p )
{
    auto base_dir = absolute(
        canonical( placement_p )
    ).parent_path();

    auto result = parse( base_dir );
    auto places = read_places( base_dir / "places" );

    auto placement = read_placement(
        placement_p ,
        places
    );

    std::cout << placement.id
                << " "
                << placement.score
                << " "
                << placement.desc
                << std::endl;
}

void peek( std::filesystem::path root_p )
{
    auto filename = generate_filename_from_idx(
        find_last_idx(
            root_p
        )
    );

    print_placement( filename );
}

}
