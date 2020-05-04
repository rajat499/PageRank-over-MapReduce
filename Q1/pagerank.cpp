
#include "include/mapreduce.hpp"
#include "graph.hpp"

#include <boost/config.hpp>
#include <chrono>

//Parameters to improve performance as per library documentation and examples
#if defined(BOOST_MSVC)
    #pragma warning(disable : 4127)

    // turn off checked iterators to avoid performance hit
        #if !defined(__SGI_STL_PORT) && !defined(_DEBUG)
        #define _SECURE_SCL 0
        #define _HAS_ITERATOR_DEBUGGING 0
    #endif
#endif

// Algorithm Constants
double ALPHA = 0.85;
double LIMIT = 1e-13;

// Size containers
int graph_size = 0, list_size = 0;

namespace pagerank
{
    //The graph of pages to operate on stored in form of vector of link
    graph_item *graph_;
    //Current rank of pages computed in previous iteration
    double *rank_;
    //Rank computed in current iteration
    double *new_rank_;

    template <typename MapTask>
    //Data source for mapreduce library
    class datasource : mapreduce::detail::noncopyable
    {
    public:
        datasource() : sequence_(0)
        {
        }

        //Settting up of key for map task
        bool const setup_key(typename MapTask::key_type &key)
        {
            key = sequence_++;
            return key < 1;
        }

        //Setting up value of corresponding key for map task
        bool const get_data(typename MapTask::key_type &key, typename MapTask::value_type &value)
        {
            value = list_size;
            return true;
        }

    //To define number of keys
    private:
        unsigned sequence_;
    };

    //Map task of the library
    // Works on individual graph_item and emits key value pairs out.
    struct map_task : public mapreduce::map_task<unsigned, unsigned>
    {
        template <typename Runtime>
        void operator()(Runtime &runtime, key_type const &key, value_type const &value) const
        {
            //Generates key value pair for reduce task to work on
            // Processing all graph_items (in range) of current map key
            for(auto i = key; i < value; i++ ){
                
                // Extracting Information
                typename Runtime::reduce_task_type::value_type v = graph_[i].first;
                typename Runtime::reduce_task_type::key_type a = graph_[i].second.first;
                typename Runtime::reduce_task_type::key_type b =  graph_[i].second.second;
                
                // Accomodating rank
                v = rank_[a] * v;
                // Avoid 0 rank
                if(v == 0) continue;
                
                // Boundary case
                if(b == -2) // DANGLING
                    b = graph_size;
                else if(b == -1) // RANDOM JUMP
                    b = graph_size + 1;

                //Emit the key value pair for reduce task
                runtime.emit_intermediate(b, v);
            }
        }
    };

    // Reduce Task Function.
    // Works on Key-MultiValue pairs from map task
    // Finally emits (node, rank) as key value pair 
    struct reduce_task : public mapreduce::reduce_task<unsigned, double>
    {
        template <typename Runtime, typename It>
        void operator()(Runtime &runtime, key_type const &key, It it_start, It it_end) const
        {
            // Extract Key
            int index = key;
            
            if(it_start == it_end)
                return;
            else{
                // Local container
                double total = 0;

                // Working with all values
                for(It it = it_start; it != it_end; ++it) 
                    total += *it;
                
                // Adding to vector (case wise)
                if(index == graph_size+1) // RANDOM
                    for(int i=0; i<graph_size; i++)
                        new_rank_[i] += total * (1 - ALPHA);
                else if(index == graph_size) // DANGLING
                    for(int i=0; i<graph_size; i++)
                        new_rank_[i] += total * ALPHA;
                else                    // REGULAR LINK
                    new_rank_[index] += total * ALPHA;
            }
            //Emit key value pairs
            runtime.emit(key, rank_[index]);
        }
    };

    //Mapreduce job for this pagerank
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
    // Reading Graph
    Graph(argv[1]).convert( &pagerank::graph_, &graph_size, &list_size);

    // Creating rank containers
    pagerank::rank_ = new double[graph_size](); 
    pagerank::rank_[0] = 1;
    pagerank::new_rank_ = new double[graph_size]();

    //Specifications of mapreduce task
    mapreduce::specification spec;
    spec.map_tasks = 4;
    spec.reduce_tasks = 4;
        

    auto start =  std::chrono::high_resolution_clock::now();
    while(1) { // PAGERANK LOOP

        // Map reduce and Pagerank entities
        pagerank::job::datasource_type datasource;
        pagerank::job job(datasource, spec);
        mapreduce::results result;
        //Running the required mapreduce job
        job.run<mapreduce::schedule_policy::cpu_parallel<pagerank::job>>(result);    
        
        // Breakcheck
        if(difference(pagerank::new_rank_, pagerank::rank_) < LIMIT) {
            if(*argv[2] == '1')
                cerr << "BREAKPOINT!" << endl;
            break;
        }

        // RESET
        for(int i=0; i<graph_size; i++) {
            pagerank::rank_[i] = pagerank::new_rank_[i];
            pagerank::new_rank_[i] = 0;
        }

    }
    auto end = std::chrono::high_resolution_clock::now();
    //If set to print output on terminal
    if(*argv[2] == '1'){
        // Logging
        cerr << "Map Reduce Complete. Time " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " milliseconds." <<endl;

        // Writing Output
        double sum = 0;
        for(int i=0; i<graph_size; i++) {
            sum += pagerank::rank_[i];
            cout << i << " : " << pagerank::rank_[i] << endl;
        }
        cerr << sum << endl;
        cerr << "DIFF: " << difference(pagerank::new_rank_, pagerank::rank_) << endl;
    }  
}
