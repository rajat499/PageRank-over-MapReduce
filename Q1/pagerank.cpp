
#include "include/mapreduce.hpp"
#include "graph.hpp"

#include <boost/config.hpp>
#include <chrono>

#if defined(BOOST_MSVC)
    #pragma warning(disable : 4127)

    // turn off checked iterators to avoid performance hit
        #if !defined(__SGI_STL_PORT) && !defined(_DEBUG)
        #define _SECURE_SCL 0
        #define _HAS_ITERATOR_DEBUGGING 0
    #endif
#endif

double ALPHA = 0.85;
int graph_size = 0, list_size = 0;
double LIMIT = 1e-13;

namespace pagerank
{
    graph_item *graph_;
    double *rank_;
    double *new_rank_;

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
            return key < 1;
        }

        bool const get_data(typename MapTask::key_type &key, typename MapTask::value_type &value)
        {
            value = list_size;
            return true;
        }

    private:
        unsigned sequence_;
    };

    struct map_task : public mapreduce::map_task<unsigned, unsigned>
    {
        template <typename Runtime>
        void operator()(Runtime &runtime, key_type const &key, value_type const &value) const
        {
            for(auto i = key; i < value; i++ ){
                typename Runtime::reduce_task_type::value_type v = graph_[i].first;
                typename Runtime::reduce_task_type::key_type a = graph_[i].second.first;
                typename Runtime::reduce_task_type::key_type b =  graph_[i].second.second;

                v = rank_[a] * v;

                if(v == 0) continue;
                
                if(b == -2) // DANGLING
                    b = graph_size;
                else if(b == -1) // RANDOM JUMP
                    b = graph_size + 1;

                runtime.emit_intermediate(b, v);
            }
        }
    };

    struct reduce_task : public mapreduce::reduce_task<unsigned, double>
    {
        template <typename Runtime, typename It>
        void operator()(Runtime &runtime, key_type const &key, It it_start, It it_end) const
        {
            
            int index = key;
            
            if(it_start == it_end)
                return;
            else{
        
                double total = 0;

                for(It it = it_start; it != it_end; ++it) 
                    total += *it;
                
                if(index == graph_size+1)
                    for(int i=0; i<graph_size; i++)
                        new_rank_[i] += total * (1 - ALPHA);
                else if(index == graph_size)
                    for(int i=0; i<graph_size; i++)
                        new_rank_[i] += total * ALPHA;
                else
                    new_rank_[index] += total * ALPHA;
            }
            
            runtime.emit(key, rank_[index]);
        }
    };

    typedef mapreduce::job<pagerank::map_task,
                        pagerank::reduce_task,
                        mapreduce::null_combiner,
                        pagerank::datasource<pagerank::map_task>>
    job;

} // namespace pagerank


/*
 * Euclidean difference b/w vectors
 * @param (_1, _2): vectors
 */
double difference(double *_1, double *_2) {
    double diff = 0;
    for(size_t i{}; i<graph_size; i++)
        diff += pow(_1[i] - _2[i], 2);
    return diff;
}


int main(int argc, char **argv)
{

    Graph(argv[1]).convert( &pagerank::graph_, &graph_size, &list_size);

    pagerank::rank_ = new double[graph_size](); 
    pagerank::rank_[0] = 1;
    pagerank::new_rank_ = new double[graph_size]();

    mapreduce::specification spec;
    
    auto start =  std::chrono::high_resolution_clock::now();
    while(1) {

        pagerank::job::datasource_type datasource;
        pagerank::job job(datasource, spec);
        mapreduce::results result;
        job.run<mapreduce::schedule_policy::cpu_parallel<pagerank::job>>(result);    
        

        if(difference(pagerank::new_rank_, pagerank::rank_) < LIMIT) {
            cerr << "BREAKPOINT!" << endl;
            break;
        }

        for(int i=0; i<graph_size; i++) {
            pagerank::rank_[i] = pagerank::new_rank_[i];
            pagerank::new_rank_[i] = 0;
        }

    }
    auto end = std::chrono::high_resolution_clock::now();
    cerr<<"Finished Page rank in: "<< std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " milliseconds" <<endl;
    double sum = 0;
    for(int i=0; i<graph_size; i++) {
        sum += pagerank::rank_[i];
        cout << i << " : " << pagerank::rank_[i] << endl;
    }
    cerr << sum << endl;
    cerr << "DIFF: " << difference(pagerank::new_rank_, pagerank::rank_) << endl;
    
}
