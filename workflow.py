from gwf import Workflow

gwf = Workflow(defaults={'account': 'phasetype'})

for i in range(3, 151):

    sentinel_file_name = f'steps/block_coal_rewards_{i}.txt'
    gwf.target(f'coal_rewards_{i}', inputs=[], outputs=[sentinel_file_name], walltime='6-23:00:00', memory = '8gb') << f"""
    mkdir -p steps
    ~/miniconda3/envs/phasetype/bin/Rscript ./scripts/coal_block_rewards.r ./data/coal_block_rewards.h5 {i} && touch {sentinel_file_name}
    """
    