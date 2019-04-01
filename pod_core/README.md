# pod_core

-e ecc_pub.bin -m table -a vrf_query -p data -o output -k first_name -v Kathy

-e ecc_pub.bin -m table -a ot_vrf_query -p data -o output -k first_name -v Kathy John abc -n dde aaa eee bbb

-e ecc_pub.bin -m plain -a range_pod -p plain_data -o plain_output -d 0-10

-e ecc_pub.bin -m plain -a ot_range_pod -p plain_data -o plain_output -d 0-10 -g 0-20

-e ecc_pub.bin -m table -a ot_batch_pod -p data -o output --demand_ranges 1-2 --phantom_ranges 0-3
