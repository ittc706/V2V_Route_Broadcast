#pragma once
#include<iostream>
#include<vector>
#include<list>
#include<queue>
#include<set>
#include<utility>
#include<random>
#include<string>
#include"route.h"
#include"config.h"
#include"reflect/object.h"
#include"reflect/context.h"


enum route_response_state{
	ACCEPT,
	REJECT
};

enum route_tcp_pattern_state {
	IDLE = -3,//����״̬
	TO_BE_SEND = -2,//��Ҫ����
	TO_BE_RECEIVE = -1,//��Ҫ����
	SENDING = 1,//����״̬
	RECEIVING = 2,//����״̬
};


/*
* ����·�ɲ㣬���漰����������������Ϊnode
* ����һ���ڵ㣬�շ�ì�ܡ���ͬһʱ��ֻ���գ�����ֻ�ܷ�
* ��һ���ڵ㴦���շ�״̬ʱ���ؾ�һ������
* ���ڵ㴦����״̬ʱ���ýڵ����ΪԴ�ڵ����ϢҲ��������Ϻ��ٽ��з��ͣ�������ת����Ϣ֮��
* ���ڵ㴦�ڿ���״̬����ͬʱ�յ���������ת���������Ӧ��һ�����ܾ�����(������չΪ���ȼ�Ӧ��)
*/

class route_tcp_node;

class route_tcp_route_event {
private:
	static int s_event_count;

	/*
	* Դ�ڵ�
	*/
private:
	const int m_origin_source_node_id;
public:
	int get_origin_source_node_id() { 
		return m_origin_source_node_id;
	}

	/*
	* Ŀ�Ľڵ�
	*/
private:
	const int m_final_destination_node_id;
public:
	int get_final_destination_node_id() { 
		return m_final_destination_node_id;
	}

	/*
	* ��ǰ�ڵ�(����ȷ���䵽��ǰ�ڵ㣬����ǰ�ڵ�Ҳ��m_through_node_vec֮��)
	*/
private:
	int m_current_node_id = -1;
public:
	void set_current_node_id(int t_current_node_id) { 
		m_current_node_id = t_current_node_id;
		m_through_node_id_vec.push_back(m_current_node_id);
	}
	int get_current_node_id() { 
		return m_current_node_id;
	}

	/*
	* �����Ľڵ��б�(ֻ�����ɹ������)
	*/
private:
	std::vector<int> m_through_node_id_vec;
public:
	const std::vector<int>& get_through_node_vec() {
		return m_through_node_id_vec;
	}


	/*
	* ������Դ�ڵ㵽Ŀ�Ľڵ��Ƿ񴫵ݳɹ�
	*/
	bool is_finished() { 
		return m_current_node_id == m_final_destination_node_id; 
	}

	/*
	* �¼�id
	*/
private:
	const int m_event_id;
public:
	int get_event_id() { return m_event_id; }

	/*
	* ���ݰ�����
	* ��Ҫ���͵����ݷ�װ��IP���ݰ�����������ЩIP���ݰ���ʧ����һ������ô�������ݰ����㶪ʧ
	*/
private:
	const int m_package_num;
public:
	int get_package_num() {
		return m_package_num;
	}

public:
	/*
	* ���캯�����ṩ���¼�����ģ�����
	*/
	route_tcp_route_event(int t_source_node, int t_destination_node) :
		m_event_id(s_event_count++), 
		m_origin_source_node_id(t_source_node),
		m_final_destination_node_id(t_destination_node),
		m_package_num(((tmc_config*)context::get_context()->get_bean("tmc_config"))->get_package_num()){
		set_current_node_id(t_source_node);
	}

	/*
	* ������ǰ�¼�������ǰ��event_id����
	* ��ĳһ���ɹ����䣬���Ǳ�ǳɹ����������ʧ�ܣ������ɶ�����·����ʱ�Ż����clone
	*/
	void transfer_to(int t_node_id) {
		set_current_node_id(t_node_id);
	}

	/*
	* תΪ�ַ���
	*/
	std::string to_string();
};

class route_tcp_link_event {
private:
	/*
	* ��ǰ��·Դ�ڵ�id
	*/
	const int m_source_node_id;
public:
	int get_source_node_id() {
		return m_source_node_id;
	}

	/*
	* ��ǰ��·Ŀ�Ľڵ�id
	*/
private:
	const int m_destination_node_id;
public:
	int get_destination_node_id() {
		return m_destination_node_id;
	}

	/*
	* ���ݰ�����
	* ��Ҫ���͵����ݷ�װ��IP���ݰ�����������ЩIP���ݰ���ʧ����һ������ô�������ݰ����㶪ʧ
	*/
private:
	const int m_package_num;

	/*
	* ռ�õ�pattern���
	*/
private:
	int m_pattern_idx;
	void set_pattern_idx(int t_pattern_idx) {
		m_pattern_idx = t_pattern_idx;
	}
public:
	int get_pattern_idx() {
		return m_pattern_idx;
	}

	/*
	* ��Ǳ�����ǰʱ�̴���İ����
	*/
private:
	int m_package_idx = 0;
public:
	int get_package_idx() { return m_package_idx; }

	/*
	* ��Ǳ����Ƿ������(�����Ƿ�������)
	*/
private:
	bool m_is_finished = false;
public:
	bool is_finished() { return m_is_finished; }

	/*
	* ��Ǳ����Ƿ�������
	*/
private:
	bool m_is_loss = false;
public:
	bool get_is_loss() { return m_is_loss; }

public:
	route_tcp_link_event(int t_source_node_id, int t_destination_node_id, int t_pattern_idx, int t_package_num) :
		m_source_node_id(t_source_node_id),
		m_destination_node_id(t_destination_node_id),
		m_pattern_idx(t_pattern_idx),
		m_package_num(t_package_num) {}

	/*
	* �������ݰ��ķ���
	*/
	void transimit();
};

class route_tcp_node {
	friend class route_tcp;

private:
	/*
	* �ڵ���
	*/
	static int s_node_count;

	/*
	* ���������
	*/
	static std::default_random_engine s_engine;

	/*
	* ���ڷ���(ǿ��һ��:��״̬�Ľڵ�)��node�ڵ�
	* ����±�Ϊpattern���
	*/
	static std::vector<std::set<int>> s_node_id_per_pattern;
public:
	static const std::set<int>& get_node_id_set(int t_pattern_idx);

	/*
	* ��ǰ�ڵ�����ͳ�������
	*/
private:
	std::queue<route_tcp_route_event*> m_send_event_queue;
public:
	void offer_send_event_queue(route_tcp_route_event* t_event) {
		m_send_event_queue.push(t_event);
	}
	route_tcp_route_event* poll_send_event_queue() {
		route_tcp_route_event* temp = m_send_event_queue.front();
		m_send_event_queue.pop();
		return temp;
	}
	route_tcp_route_event* peek_send_event_queue() {
		return m_send_event_queue.front();
	}
	bool is_send_event_queue_empty() {
		return m_send_event_queue.empty();
	}

private:
	/*
	* ���ֳɹ��󣬴���link_event����ӵ�relay�ڵ�ĸýṹ�У������¸�tti����
	*/
	std::vector<route_tcp_link_event*> m_next_round_link_event;

	/*
	* ��ǰ�ڵ㣬��ǰʱ�̣�ÿ��pattern��ʹ�����
	* ��pair��first�ֶΣ���pattern״̬ΪIDLE����ô�ڲ�pair��second�ֶ���Ч
	* pair��second�ֶ�Ϊ��·�¼�ָ��
	*/
private:
	std::vector<std::pair<route_tcp_pattern_state, route_tcp_link_event*>> m_pattern_state;

	/*
	* ��ǰ�ڵ㣬�ϸ�tti��ÿ��Ƶ�����յ���������������syn����
	*/
private:
	std::vector<std::vector<int>> m_last_round_request_per_pattern;

private:
	/*
	* ����tti�յ���syn���󣬽����¸�tti����ack����
	*/
	std::vector<std::vector<int>> m_syn_request_per_pattern;
public:
	//���syn����
	void add_syn_request(int t_pattern_idx, int t_source_node_id) {
		m_syn_request_per_pattern[t_pattern_idx].push_back(t_source_node_id);
	}

	/*
	* (-1,-1)״̬˵����Ҫ����sync
	* �ڷ���synʱ�����и�ֵ�������ڱ�relay�ڵ�reject�����ã��ڴ�����Ϻ�����
	* ����һ���ڵ���ͬһ��ʱ��ֻ�ܷ���һ���¼�����˲��÷�pattern
	*/
private:
	std::pair<int, int> m_select_cache;
public:
	//�Ƿ��Ѿ�����syn����syn�Ƿ���Ч
	bool is_already_send_syn() {
		return m_select_cache.first != -1 && m_select_cache.second != -1;
	}
	//����syn״̬
	void reset_syn_state() {
		m_select_cache.first = -1;
		m_select_cache.second = -1;
	}

private:
	/*
	* �ڵ�id
	*/
	const int m_id = s_node_count++;
public:
	int get_id() {
		return m_id;
	}

	/*
	* �ڽ��б�
	*/
private:
	std::vector<int> m_adjacent_list;
public:
	void add_to_adjacent_list(int t_node_id) {
		m_adjacent_list.push_back(t_node_id);
	}
	const std::vector<int>& get_adjacent_list() {
		return m_adjacent_list;
	}


public:
	/*
	* ���캯��
	*/
	route_tcp_node();

public:
	/*
	* ѡ������ת���ĳ����Լ���Ӧ��Ƶ��
	* first�ֶ�Ϊ����id
	* second�ֶ�ΪƵ�α��
	* ����һ���ֶ�Ϊ-1���ʹ���ѡ��ʧ��
	*/
	std::pair<int,int> select_relay_information();
};

class v2x_time;
class gtt;
class wt;
class rrm_config;
class tmc_config;
class route_config;

class route_tcp :public route {
	REGISTE_MEMBER_HEAD(route_tcp)

private:
	/*
	* ���������
	*/
	static std::default_random_engine s_engine;

	/*
	* ��־�����
	*/
	static std::ofstream s_logger_pattern;
	static std::ofstream s_logger_link;
	static std::ofstream s_logger_event;

	/*
	* ��¼��־
	*/
	static void log_node_pattern(int t_source_node_id, 
		int t_relay_node_id, 
		int t_cur_node_id, 
		int t_pattern_idx, 
		route_tcp_pattern_state t_from_pattern_state, 
		route_tcp_pattern_state t_to_pattern_state, 
		std::string t_description);

	static std::string pattern_state_to_string(route_tcp_pattern_state t_pattern_state);

	static void log_event(int t_origin_node_id, int t_fianl_destination_node_id);

	static void log_link(int t_source_node_id, int t_relay_node_id, std::string t_description);
private:
	/*
	* �ڵ�����
	*/
	route_tcp_node* m_node_array;
public:
	route_tcp_node* get_node_array() { 
		return m_node_array; 
	}

private:
	/*
	* �ɹ�/ʧ�ܴ�����¼�
	*/
	std::vector<route_tcp_route_event*> m_successful_event_vec;
	std::vector<route_tcp_link_event*> m_failed_event_vec;
	void add_successful_event(route_tcp_route_event* t_successful_event_vec) {
		m_successful_event_vec.push_back(t_successful_event_vec);
	}
	void add_failed_event(route_tcp_link_event* t_failed_event_vec) {
		m_failed_event_vec.push_back(t_failed_event_vec);
	}
public:
	const std::vector<route_tcp_route_event*>& get_successful_event_vec() {
		return m_successful_event_vec;
	}
	const std::vector<route_tcp_link_event*>& get_failed_event_vec(){
		return m_failed_event_vec;
	}

private:
	v2x_time* m_time;
	gtt* m_gtt;
	wt* m_wt;
	rrm_config* m_rrm_config;
	tmc_config* m_tmc_config;
	route_config* m_route_config;

	void set_time(object* t_time) {
		m_time = (v2x_time*)t_time;
	}
	void set_gtt(object* t_gtt) {
		m_gtt = (gtt*)t_gtt;
	}
	void set_wt(object* t_wt) {
		m_wt = (wt*)t_wt;
	}
	void set_rrm_config(object* t_rrm_config) {
		m_rrm_config = (rrm_config*)t_rrm_config;
	}
	void set_tmc_config(object* t_tmc_config) {
		m_tmc_config = (tmc_config*)t_tmc_config;
	}
	void set_route_config(object* t_route_config) {
		m_route_config = (route_config*)t_route_config;
	}
public:
	v2x_time* get_time() override {
		return m_time;
	}

	gtt* get_gtt() override {
		return m_gtt;
	}

	wt* get_wt() override {
		return m_wt;
	}

	rrm_config* get_rrm_config() override {
		return m_rrm_config;
	}

	tmc_config* get_tmc_config() override {
		return m_tmc_config;
	}

	route_config* get_route_config() override {
		return m_route_config;
	}

	void initialize() override;

	void process_per_tti() override;

	void update_route_table_from_physics_level() override;

private:
	/*
	* ��������¼�
	*/
	void event_trigger();

	/*
	* ����syn
	*/
	void send_syn();

	/*
	* ����ack
	*/
	void send_ack();

	/*
	* ���н���(�շ����ˣ������ն���Ϊ����㣬�������ն˴����ͬʱһ������)
	*/
	void receive_data();

	/*
	* ��tti��ʼʱ��ˢ��tobe���ݽṹ
	*/
	void update_tobe();
};



