#ifndef _MINIBATCH_TRAINER_
#define _MINIBATCH_TRAINER_

template<class GRADIENT_CLASS>
class MinibatchTrainer : public Trainer<GRADIENT_CLASS> {
public:
    MinibatchTrainer() {}
    ~MinibatchTrainer() {}

    TrainStatistics Train(Model *model, const std::vector<Datapoint *> & datapoints, Updater<GRADIENT_CLASS> *updater) override {
	// Partition.
	BasicPartitioner partitioner;
	Timer partition_timer;
	DatapointPartitions partitions = partitioner.Partition(datapoints, FLAGS_n_threads);
	if (FLAGS_print_partition_time) {
	    this->PrintPartitionTime(partition_timer);
	}

	model->SetUpWithPartitions(partitions);

	// Keep track of statistics of training.
	TrainStatistics stats;

	// Train.
	Timer gradient_timer;
	for (int epoch = 0; epoch < FLAGS_n_epochs; epoch++) {

	    this->EpochBegin(epoch, gradient_timer, model, datapoints, &stats);

	    updater->EpochBegin();

#pragma omp parallel for schedule(static, 1)
	    for (int thread = 0; thread < FLAGS_n_threads; thread++) {
		for (int batch = 0; batch < partitions.NumBatches(); batch++) {
		    updater->UpdateMultiple(model, partitions, batch, thread);
		}
	    }
	    model->EpochFinish();
	}
	return stats;
    }
};

#endif
