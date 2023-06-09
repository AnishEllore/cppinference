#include <iostream>
#include <map>
#include <fstream>
#include <string>
#include <sstream>
#include <cstring>
#include <vector>
#include <istream>

#include <torch/torch.h>
#include <torch/script.h>



std::pair<torch::Tensor, torch::Tensor> preprocess(std::string text, std::map<std::string, int> token2id, int max_length, bool log = false){
    std::string pad_token = "[PAD]", start_token = "[CLS]", end_token = "[SEP]";
    int pad_token_id = token2id[pad_token], start_token_id = token2id[start_token], end_token_id = token2id[end_token];

    std::vector<int> input_ids(max_length, pad_token_id), masks(max_length, 0);
    input_ids[0] = start_token_id; masks[0] = 1;
    //std::cout<<"log1"<<std::endl;
    std::string word;
    std::istringstream ss(text);
    //std::cout<<"log2"<<std::endl;
    int input_id = 1;
    while(getline(ss, word, ' ')) {
        int word_id = token2id[word];
        masks[input_id] = 1;
        input_ids[input_id++] = word_id;
        
        if (log)
            std::cout << word << " : " << word_id << '\n';
    }
    //std::cout<<"log3"<<std::endl;
    masks[input_id] = 1;
    input_ids[input_id] = end_token_id;

    if (log){
        for (auto i : input_ids)
            std::cout << i << ' ';
        std::cout << '\n';
    
        for (auto i : masks)
            std::cout << i << ' ';
        std::cout << '\n';
    }
    //std::cout<<"log4"<<std::endl;
    auto input_ids_tensor = torch::tensor(input_ids).unsqueeze(0);
    auto masks_tensor = torch::tensor(masks).unsqueeze(0).unsqueeze(0);
    //std::cout<<"log5"<<std::endl;
    return std::make_pair(input_ids_tensor, masks_tensor);
}



struct Model{
    int max_length = 32;
    std::map<std::string, int> token2id;
    std::map<int, std::string> id2token;
    torch::jit::script::Module bert;
    std::map<int, std::string> pred2class = {{0, "hate-speech"}, {1, "offensive-language"}, {2, "neither"}};
    Model() {
        c10::InferenceMode guard;
        init_vocab();
        init_bert();
    }
    // disable copy constructor
    //Model(const Model&) = delete;
    // print when something is deleted
    ~Model() {
        std::cout << "Model deleted" << std::endl;
    }
    // print when something is moved
    Model(Model&&) {
        std::cout << "Model moved" << std::endl;
    }
    // print when copy assignment happens
    Model& operator=(const Model&) {
        std::cout << "Model copy-assigned" << std::endl;
        return *this;
    }
    private:
        void init_vocab(std::string vocab_path = "../model/bert_cased_vocab.txt"){
            std::tie(token2id, id2token) = get_vocab(vocab_path);
        }



        void init_bert(std::string bert_path = "../model/traced_text_classification_model.pt"){
            bert = load_model(bert_path);
            bert.eval();
        }

        std::pair<std::map<std::string, int>, std::map<int, std::string>> get_vocab(std::string vocab_path){
            std::map<std::string, int> token2id;
            std::map<int, std::string> id2token;

            std::fstream newfile;
            newfile.open(vocab_path, std::ios::in);

            std::string line;
            while(getline(newfile, line)){
                char *token = strtok(const_cast<char*>(line.c_str()), " ");
                char *token_id = strtok(nullptr, " ");

                token2id[token] = std::stoi(token_id);
                id2token[std::stoi(token_id)] = token;
            }
            newfile.close();

            return std::make_pair(token2id, id2token);
        }

        torch::jit::script::Module load_model(std::string  model_path){
            torch::jit::script::Module module;
            try {
                module = torch::jit::load(model_path);
            }
            catch (const c10::Error& e) {
                std::cerr << "error loading the model\n";
            }
            return module;
    }
};


Model globalModel = Model();
std::string infer(const std::string& input) {
    c10::InferenceMode guard;
    //std::cout << "start" << std::endl;
    
    //std::string text = std::string(input_view);

    //set seed
    int seed = 42;
    torch::manual_seed(seed);
    //torch::cuda::manual_seed(seed);



    auto token2id = globalModel.token2id;
    torch::Tensor input_ids, masks;
    std::tie(input_ids, masks) = preprocess(input, token2id, globalModel.max_length);
    //std::cout << "preprocess" << std::endl;
    std::vector<torch::jit::IValue> inputs;
    inputs.push_back(input_ids);
    inputs.push_back(masks);

    auto outputs = globalModel.bert.forward(inputs).toTensor();
    std:: string result = std::to_string(int(outputs.argmax().item<int>()));
    //std::string result = model.pred2class[int(outputs.argmax().item<int>())];
    //std::cout << "Prediction: " << result  << std::endl;
    return result;
}

