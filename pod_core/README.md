# pod_core

-e ecc_pub.bin -m plain -a range_pod -p plain_data -o plain_output -d 0-10

-e ecc_pub.bin -m plain -a ot_range_pod -p plain_data -o plain_output -d 0-10 -g 0-20

-e ecc_pub.bin -m table -a ot_batch_pod -p table_data -o output --demand_ranges 1-2 --phantom_ranges 0-3

-e ecc_pub.bin -m table -a batch_pod -p table_data -o table_output --demand_ranges 1-2

-e ecc_pub.bin -m table -a batch2_pod -p table_data -o table_output --demand_ranges 1-2

-e ecc_pub.bin -m table -a vrf_query -p table_data -o table_output -k first_name -v Kathy

-e ecc_pub.bin -m table -a ot_vrf_query -p table_data -o table_output -k first_name -v Kathy John abc -n dde aaa eee bbb
