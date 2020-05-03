// Copyright (c) 2009-2016 Craig Henderson
// https://github.com/cdmh/mapreduce

#include <boost/config.hpp>
#if defined(BOOST_MSVC)
#pragma warning(disable : 4127)

// turn off checked iterators to avoid performance hit
#if !defined(__SGI_STL_PORT) && !defined(_DEBUG)
#define _SECURE_SCL 0
#define _HAS_ITERATOR_DEBUGGING 0
#endif
#endif
#include "include/mapreduce.hpp"
#include <chrono>

namespace pagerank
{
std::map<unsigned, std::vector<std::pair<unsigned, double>>> hlm;
double *aai;
unsigned maxsize;
template <typename MapTask>
class datasource : mapreduce::detail::noncopyable
{
public:
    datasource() : sequence_(0)
    {
    }

    bool const setup_key(typename MapTask::key_type &key)
    {
        key = sequence_++;
        return key < maxsize;
    }

    bool const get_data(typename MapTask::key_type const &key, typename MapTask::value_type &value)
    {
        typename MapTask::value_type val;
        val = hlm[key];
        std::swap(val, value);
        return true;
    }

private:
    unsigned sequence_;
};

struct map_task : public mapreduce::map_task<unsigned, std::vector<std::pair<unsigned, double>>>
{
    template <typename Runtime>
    void operator()(Runtime &runtime, key_type const &key, value_type const &value) const
    {
        typename Runtime::reduce_task_type::key_type const emit_key = key;
        typename Runtime::reduce_task_type::value_type val = value;
        runtime.emit_intermediate(emit_key, val);
    }
};

struct reduce_task : public mapreduce::reduce_task<unsigned, std::vector<std::pair<unsigned, double>>>
{
    template <typename Runtime, typename It>
    void operator()(Runtime &runtime, key_type const &key, It it, It ite) const
    {
        std::vector<std::pair<unsigned, double>> ans;
        if (it == ite)
        {
            ans.push_back(std::make_pair(key, 0));
            runtime.emit(key, ans);
            return;
        }

        else if (std::distance(it, ite) == 1)
        {
            std::vector<std::pair<unsigned, double>> vec = *it;
            double ans2 = 0;
            for (int i = 0; i < vec.size(); i++)
            {
                std::pair<unsigned, double> p = vec[i];
                ans2 += (pagerank::aai[p.first]) * p.second;
            }
            ans.push_back(std::make_pair(key, ans2));
            runtime.emit(key, ans);
            return;
        }
    }
};

typedef mapreduce::job<pagerank::map_task,
                       pagerank::reduce_task,
                       mapreduce::null_combiner,
                       pagerank::datasource<pagerank::map_task>>
    job;

} // namespace pagerank


int main(int argc, char *argv[])
{
    auto start = std::chrono::high_resolution_clock::now();
    mapreduce::specification spec;

    std::string infile = argv[1];
    double alpha = std::stod(argv[2]);
    std::fstream myfile(infile);

    unsigned a, b, maxa = 0;
    std::map<unsigned, std::vector<std::pair<unsigned, double>>> m;
    for (int i = 0; i <= maxa; i++)
    {
        std::vector<std::pair<unsigned, double>> v;
        m[i] = v;
    }

    while (myfile >> a)
    {
        maxa = std::max(maxa, a);
        myfile >> b;
        maxa = std::max(maxa, b);
        // std::cout << a << " " << b << "\n";
    }
    // std::cout << maxa << std::endl;
    pagerank::maxsize = maxa + 1;

    double *edges;
    edges = new double[maxa + 1];
    for (int i = 0; i <= maxa; i++)
    {
        edges[i] = 0;
    }
    std::fstream myfile1(infile);
    while (myfile1 >> a)
    {
        myfile1 >> b;
        edges[a]++;
        m[b].push_back(std::make_pair(a, 1));
    }

    for (int i = 0; i <= maxa; i++)
    {
        for (int j = 0; j < m[i].size(); j++)
        {
            m[i][j].second/=edges[m[i][j].first];
        }
    }
    pagerank::hlm=m;
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    // std::cout << "Duration in Reading and creating H : "<<duration.count()<<"\n";

    //Initialisation of I
    pagerank::aai = new double[maxa + 1];
    for (int i = 0; i <= maxa; i++)
    {
        pagerank::aai[i] = 0;
    }
    pagerank::aai[0] = 1;
    //Done

    //final algorithm
    int itr = 60;
    std::vector<double> prev(maxa + 1);
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 61; i++)
    {
        pagerank::job::datasource_type datasource;
        pagerank::job job(datasource, spec);
        mapreduce::results result;
        job.run<mapreduce::schedule_policy::cpu_parallel<pagerank::job>>(result);
        unsigned k = 0;
        double sum = 0, sum0 = 0;
        for (auto it = job.begin_results(); it != job.end_results(); ++it)
        {
            double xx = pagerank::aai[k];
            prev[k] = xx;
            if (edges[k] == 0)
            {
                sum0 += xx;
            }
            sum += xx;
            pagerank::aai[k] = (it->second[0].second);
            k++;
        }
        sum /= (maxa + 1);
        sum0 /= (maxa + 1);
        double add = alpha * sum0 + (1 - alpha) * sum;
        double error = 0;

        for (int r = 0; r <= maxa; r++)
        {
            pagerank::aai[r] = alpha * (pagerank::aai[r]) + add;
            error += std::abs(pagerank::aai[r] - prev[r]);
        }
        if (error < 0.00000001)
        {
            itr = i;
            break;
        }
    }
    // std::cout << "No. of iterations taken by the algorithm = "<<itr<< "\n";
    double s = 0, aa, diff;
    bool bb = true;
    std::string st;
    for (int i = 0; i <= maxa; i++)
    {
        std::cout << i << " = " << pagerank::aai[i] << "\n";
        s += pagerank::aai[i];
    }
    std::cout << "sum " << s<<"\n";
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    // std::cout << "Duration in Algorithm : "<<duration.count()<<"\n";

    return 0;
}